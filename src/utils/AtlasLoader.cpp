#include "utils/AtlasLoader.hpp"

#include <SDL.h>
#include <SDL_image.h>

#include <cctype>      // 用於檔名安全化
#include <filesystem>  // 用於處理暫存目錄和檔名
#include <fstream>     // 用於文件內容讀取
#include <sstream>     // 用於字符分割讀取
#include <stdexcept>   // 用於拋出異常
#include <string>
#include <string_view>


void AtlasLoader::loadAtlas(const std::string_view filePath) {
    atlasMap.clear();
    m_Cache.clear();
    atlasImagePath.clear();

    std::ifstream file(filePath.data());

    if (!file.is_open()) {
        throw std::runtime_error("無法打開: " + std::string(filePath));
    }

    const std::string atlasFilePath(filePath);
    const std::size_t slashPos = atlasFilePath.find_last_of("/\\");
    const std::string atlasDir =
        slashPos == std::string::npos ? "" : atlasFilePath.substr(0, slashPos + 1);

    std::string line;
    std::string currentAtlasName;
    std::string currentAtlasIndex = "None";
    Bounds B;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        if (line.find(".png") != std::string::npos) {
            atlasImagePath = atlasDir + line;
            continue;
        }

        if (line.find("size:") != std::string::npos) {
            continue;
        }

        if (line.find("filter:") != std::string::npos) {
            continue;
        }

        if (line.find("repeat:") != std::string::npos) {
            continue;
        }

        if (line.find("index:") != std::string::npos) {
            currentAtlasIndex = line.substr(line.rfind("index:") + 6);
            continue;
        }

        if (line.find("bounds:") != std::string::npos) {
            std::istringstream iss(line);
            iss.ignore(7); // Ignore "bounds:"
            char comma; // 用於忽略逗號
            iss >> B.x >> comma >> B.y >> comma >> B.w >> comma >> B.h;

            if (currentAtlasIndex == "None") {
                atlasMap[currentAtlasName] = B;
            } else {
                atlasMap[currentAtlasName + "_" + currentAtlasIndex] = B;
                currentAtlasIndex = "None";
            }
            continue;
        }

        currentAtlasName = line;
    }
}

Bounds AtlasLoader::getBounds(const std::string_view name) const {
    auto it = atlasMap.find(std::string(name));
    if (it != atlasMap.end()) {
        return it->second;
    } else {
        throw std::runtime_error("找不到: " + std::string(name));
    }
}

std::shared_ptr<Util::Image> AtlasLoader::getImage(const std::string_view name) {
    // 1. 快取命中直接回傳
    const std::string key(name);
    const auto cacheIt = m_Cache.find(key);
    if (cacheIt != m_Cache.end()) return cacheIt->second;

    // 2. 取得 bounds
    Bounds b = getBounds(name); // 若找不到會拋例外

    // 3. 載入整張 atlas 大圖
    SDL_Surface* atlasSurface = IMG_Load(atlasImagePath.c_str());
    if (!atlasSurface) {
        throw std::runtime_error("無法載入 atlas 圖片: " + atlasImagePath);
    }

    // 4. 驗證 bounds 不超出大圖範圍
    if (b.x < 0 || b.y < 0 || b.w <= 0 || b.h <= 0 ||
        b.x + b.w > atlasSurface->w || b.y + b.h > atlasSurface->h) {
        SDL_FreeSurface(atlasSurface);
        throw std::runtime_error("bounds 超出圖片範圍: " + key);
    }

    // 5. 建立與 sprite 等大的空白 surface
    SDL_Surface* spriteSurface = SDL_CreateRGBSurfaceWithFormat(
        0, b.w, b.h, 32, atlasSurface->format->format);
    if (!spriteSurface) {
        SDL_FreeSurface(atlasSurface);
        throw std::runtime_error("SDL_CreateRGBSurfaceWithFormat 失敗");
    }

    // 6. 從大圖 Blit 裁切區塊到小圖
    SDL_Rect srcRect{ b.x, b.y, b.w, b.h };
    if (SDL_BlitSurface(atlasSurface, &srcRect, spriteSurface, nullptr) != 0) {
        SDL_FreeSurface(spriteSurface);
        SDL_FreeSurface(atlasSurface);
        throw std::runtime_error("SDL_BlitSurface 失敗");
    }

    // 7. 存成暫存 BMP（Image 需要路徑）
    namespace fs = std::filesystem;
    fs::path cacheDir = fs::temp_directory_path() / "atlas_cache";
    fs::create_directories(cacheDir);

    // 安全化檔名（特殊字元換成底線）
    std::string safeName(name);
    for (char& c : safeName)
        if (!std::isalnum((unsigned char)c) && c != '_' && c != '-') c = '_';

    std::string cachePath = (cacheDir / (safeName + ".bmp")).string();
    if (SDL_SaveBMP(spriteSurface, cachePath.c_str()) != 0) {
        SDL_FreeSurface(spriteSurface);
        SDL_FreeSurface(atlasSurface);
        throw std::runtime_error("SDL_SaveBMP 失敗");
    }

    SDL_FreeSurface(spriteSurface);
    SDL_FreeSurface(atlasSurface);

    // 8. 建立 Image 物件並寫入快取
    auto img = std::make_shared<Util::Image>(cachePath);
    m_Cache[key] = img;
    return img;
}

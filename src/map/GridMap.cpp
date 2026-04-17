#include "map/GridMap.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

// -------------------- 建立地圖 --------------------
GridMap::GridMap(std::string_view MAP_FILE_PATH, AtlasLoader& atlas)
    : MAP_FILE_PATH(MAP_FILE_PATH), atlasLoader(atlas) {
    std::ifstream file(this->MAP_FILE_PATH);

    if (!file.is_open()) {
        throw std::runtime_error("無法打開地圖文件: " + std::string(MAP_FILE_PATH));
    }

    std::string line;
    bool isFirstLine = true;
    bool isSecondLine = true;
    bool isThirdLine = true;
    int heightCounter = 0; // 用於計算地圖的高度
    int widthCounter = -1;

    // -------------------- 讀取地圖資料 --------------------
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // 讀取地圖文件的前三行，分別是地圖名稱、描述和難度
        if (isFirstLine) {
            mapName = line;
            isFirstLine = false;
            continue;
        }
        if (isSecondLine) {
            mapDescription = line;
            isSecondLine = false;
            continue;
        }
        if (isThirdLine) {
            mapDifficulty = line;
            isThirdLine = false;
            continue;
        }

        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);  // 使用 istringstream 來解析每行的內容
        std::string tileType;  // 用於存儲從每行解析出的 tile 類型
        int currentRowWidth = 0;
        while (std::getline(iss, tileType, ',')) {
            if (!tileType.empty() && tileType.back() == '\r') {
                tileType.pop_back();
            }
            tilesArray.emplace_back(tileType); // emplace_back(arg): 直接用 arg 呼叫 Tile(arg) 在 vector 內原地建構
            currentRowWidth++;
        }

        if (widthCounter == -1) {
            widthCounter = currentRowWidth;
        } else if (currentRowWidth != widthCounter) {
            throw std::runtime_error("地圖每一行欄數不一致");
        }
        heightCounter++; // 每讀取一行地圖數據，增加高度計數器
    }

    if (heightCounter == 0) throw std::runtime_error("地圖文件中沒有地圖數據");
    mapHeight = heightCounter; // 最終的高度就是讀取的地圖數據行數
    mapWidth = widthCounter;

    // -------------------- 開始繪製地圖 --------------------
    tileObjects.reserve(tilesArray.size());

    const std::shared_ptr<Util::Image> firstImage = atlasLoader.getImage(tilesArray.front().getSpriteId()); // 取第一格當作基準尺寸
    const glm::vec2 firstSize = firstImage->GetSize(); // PTSD API: 取得圖片原始寬高
    constexpr float mapScale = 0.3F; // 地圖整體縮放倍率
    const float cellW = firstSize.x * mapScale; // 每格在世界座標的寬（縮放後）
    const float cellH = firstSize.y * mapScale; // 每格在世界座標的高（縮放後）
    const float startX = -(mapWidth * cellW) * 0.5F + cellW * 0.5F; // 從左邊第一格中心開始，讓整張圖置中
    const float startY = -(mapHeight * cellH) * 0.5F + cellH * 0.5F; // 從下邊第一格中心開始，讓整張圖置中

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            std::shared_ptr<Util::GameObject> obj = std::make_shared<Util::GameObject>();
            std::shared_ptr<Util::Image> image = atlasLoader.getImage(getTile(x, y).getSpriteId());
            obj->SetDrawable(image);
            obj->m_Transform.scale = {mapScale, mapScale};
            obj->m_Transform.translation = {
                startX + x * cellW, // x 方向照欄位往右排
                startY + y * cellH // world y 向上，y=0 是最下方
            };
            mapRoot.AddChild(obj);
            tileObjects.emplace_back(obj);
        }
    }
}

// -------------------- 地圖描述資訊 --------------------
std::string GridMap::getMapName() const {
    return mapName;
}

std::string GridMap::getMapDescription() const {
    return mapDescription;
}

std::string GridMap::getMapDifficulty() const {
    return mapDifficulty;
}

// -------------------- 格子查詢 --------------------
Tile GridMap::getTile(int x, int y) const {
    if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) {
        throw std::out_of_range("座標超出地圖範圍");
    }
    const int row = mapHeight - 1 - y; // world y -> 檔案 row（CSV 是由上往下）
    return tilesArray[row * mapWidth + x];
}

// -------------------- 地圖尺寸 --------------------
int GridMap::getMapWidth() const {
    return mapWidth;
}

int GridMap::getMapHeight() const {
    return mapHeight;
}

// -------------------- 規則判斷 --------------------
bool GridMap::canBuildTower(int x, int y) const {
    if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) {
        return false;
    }

    return getTile(x, y).getType() == Tile::Type::Platform;
}

bool GridMap::canWalk(int x, int y) const {
    if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) {
        return false;
    }

    Tile::Type type = getTile(x, y).getType();
    return type == Tile::Type::Road || type == Tile::Type::Spawn || type == Tile::Type::Goal;
}

// -------------------- 起終點座標 --------------------
std::vector<std::pair<int, int>> GridMap::getSpawnGridPoints() const {
    std::vector<std::pair<int, int>> spawnPoints;

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            if (getTile(x, y).getType() == Tile::Type::Spawn) {
                spawnPoints.emplace_back(x, y);
            }
        }
    }

    return spawnPoints;
}

std::optional<std::pair<int, int>> GridMap::getGoalGridPoint() const {
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            if (getTile(x, y).getType() == Tile::Type::Goal) {
                return std::pair<int, int>{x, y};
            }
        }
    }

    return std::nullopt;
}

// -------------------- 鏡頭與渲染 --------------------
void GridMap::moveCamera(float dx, float dy) {
    // 與敵人管理器保持一致：直接平移所有現有渲染物件。
    for (const std::shared_ptr<Util::GameObject>& obj : tileObjects) {
        obj->m_Transform.translation.x += dx;
        obj->m_Transform.translation.y += dy;
    }
}

void GridMap::displayMap() {
    // 由 map root 統一提交整張地圖繪製。
    mapRoot.Update();
}

#include "map/GridMap.hpp"

#include <array>
#include <cmath>
#include <fstream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

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

    // 取第一個「非 Empty」格子當作基準尺寸，避免地圖左上角是 empty 時載圖失敗。
    std::string firstSpriteId;
    for (const Tile& tile : tilesArray) {
        if (tile.getType() != Tile::Type::Empty) {
            firstSpriteId = tile.getSpriteId();
            break;
        }
    }
    if (firstSpriteId.empty()) {
        throw std::runtime_error("地圖全部都是 Empty，無法建立地圖尺寸基準");
    }

    const std::shared_ptr<Util::Image> firstImage = atlasLoader.getImage(firstSpriteId);
    const glm::vec2 firstSize = firstImage->GetSize(); // PTSD API: 取得圖片原始寬高
    m_CellW = firstSize.x * kMapScale; // 每格在世界座標的寬（縮放後）
    m_CellH = firstSize.y * kMapScale; // 每格在世界座標的高（縮放後）
    m_StartX = -(mapWidth * m_CellW) * 0.5F + m_CellW * 0.5F; // 從左邊第一格中心開始，讓整張圖置中
    m_StartY = -(mapHeight * m_CellH) * 0.5F + m_CellH * 0.5F; // 從下邊第一格中心開始，讓整張圖置中

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            const Tile& tile = getTile(x, y);
            const Tile::Type tileType = tile.getType();
            if (tileType == Tile::Type::Empty) {
                continue; // Empty 不建立渲染物件
            }
            // 當偵測到 Spawn 或 Goal 時，建立一個 Road 圖層在 Spawn / Goal 圖層下方。
            if (tileType == Tile::Type::Spawn || tileType == Tile::Type::Goal) {
                std::string roadSpriteId = "tile-type-road-"; // 預設底圖
                static constexpr std::array<std::pair<int, int>, 4> kFourDirs = {{
                    {0, 1}, {1, 0}, {0, -1}, {-1, 0}
                }};
                for (const auto [dx, dy] : kFourDirs) {
                    const int nx = x + dx;
                    const int ny = y + dy;
                    const bool inBounds = nx >= 0 && nx < mapWidth && ny >= 0 && ny < mapHeight;
                    const bool isRoad = inBounds && getTile(nx, ny).getType() == Tile::Type::Road;
                    roadSpriteId += isRoad ? "o" : "x";
                }
                std::shared_ptr<Util::GameObject> baseObj = std::make_shared<Util::GameObject>();
                std::shared_ptr<Util::Image> roadImage = atlasLoader.getImage(roadSpriteId);
                baseObj->SetDrawable(roadImage);
                baseObj->m_Transform.scale = {kMapScale, kMapScale};
                baseObj->m_Transform.translation = {
                    m_StartX + x * m_CellW, // x 方向照欄位
                    m_StartY + y * m_CellH // world y 向上，y=0 是最下方
                };
                baseObj->SetZIndex(kTileZIndex - 1.0F); // 確保在 Spawn / Goal 圖層下方
                mapRoot.AddChild(baseObj);
                tileObjects.emplace_back(baseObj);
            }

            std::shared_ptr<Util::GameObject> obj = std::make_shared<Util::GameObject>();
            std::shared_ptr<Util::Image> image = atlasLoader.getImage(tile.getSpriteId());
            obj->SetDrawable(image);
            obj->m_Transform.scale = {kMapScale, kMapScale};
            obj->m_Transform.translation = {
                m_StartX + x * m_CellW, // x 方向照欄位往右排
                m_StartY + y * m_CellH // world y 向上，y=0 是最下方
            };
            obj->SetZIndex(kTileZIndex);
            mapRoot.AddChild(obj);
            tileObjects.emplace_back(obj);
        }
    }
}

// -------------------- 地圖描述資訊 --------------------
std::string_view GridMap::getMapName() const {
    return mapName;
}

std::string_view GridMap::getMapDescription() const {
    return mapDescription;
}

std::string_view GridMap::getMapDifficulty() const {
    return mapDifficulty;
}

// -------------------- 格子查詢 --------------------
const Tile& GridMap::getTile(int x, int y) const {
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

    const Tile::Type type = getTile(x, y).getType();
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

// -------------------- 滑鼠點擊轉換世界座標 --------------------
std::optional<std::pair<int, int>> GridMap::worldToGrid(const glm::vec2& worldPos) const {
    if (m_CellW <= 0.0F || m_CellH <= 0.0F) {
        return std::nullopt;
    }

    // m_StartX / m_StartY 是「(0,0) 格中心」，先退半格得到地圖左下角邊界。
    const float leftBound = (m_StartX + m_CameraOffsetX) - (m_CellW * 0.5F); // worldPos 是世界座標，先扣掉相機偏移，在扣掉半格寬高，獲取由中心點往外的邊界位置
    const float bottomBound = (m_StartY + m_CameraOffsetY) - (m_CellH * 0.5F);
    const int gridX = static_cast<int>(std::floor((worldPos.x - leftBound) / m_CellW));
    const int gridY = static_cast<int>(std::floor((worldPos.y - bottomBound) / m_CellH));

    // 檢查座標是否在地圖範圍內
    if (gridX < 0 || gridX >= mapWidth || gridY < 0 || gridY >= mapHeight) {
        return std::nullopt;
    }

    return std::pair<int, int>{gridX, gridY};
}

std::optional<glm::vec2> GridMap::gridToWorld(int gridX, int gridY) const {
    if (gridX < 0 || gridX >= mapWidth || gridY < 0 || gridY >= mapHeight) {
        return std::nullopt;
    }

    return glm::vec2{
        m_StartX + static_cast<float>(gridX) * m_CellW + m_CameraOffsetX,
        m_StartY + static_cast<float>(gridY) * m_CellH + m_CameraOffsetY
    };
}

// -------------------- 鏡頭與渲染 --------------------
void GridMap::moveCamera(float dx, float dy) {
    // 與敵人管理器保持一致：直接平移所有現有渲染物件。
    m_CameraOffsetX += dx;
    m_CameraOffsetY += dy;
    for (const std::shared_ptr<Util::GameObject>& obj : tileObjects) {
        obj->m_Transform.translation.x += dx;
        obj->m_Transform.translation.y += dy;
    }
}

void GridMap::displayMap() {
    // 由 map root 統一提交整張地圖繪製。
    mapRoot.Update();
}

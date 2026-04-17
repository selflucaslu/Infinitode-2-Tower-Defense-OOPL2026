#include "map/GridMap.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>


GridMap::GridMap(std::string_view MAP_FILE_PATH, std::shared_ptr<AtlasLoader> atlas)
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
    mapHeight = heightCounter;
    mapWidth = widthCounter;

    // -------------------- 開始繪製地圖 --------------------
    tileObjects.reserve(tilesArray.size());

    const std::shared_ptr<Util::Image> firstImage = this->atlasLoader->getImage(tilesArray.front().getSpriteId());
    const glm::vec2 firstSize = firstImage->GetSize();

    // 初始化基準寬高與相機狀態
    baseCellWidth = firstSize.x;
    baseCellHeight = firstSize.y;
    currentScale = 1.0F;
    cameraX = 0.0F;
    cameraY = 0.0F;

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            std::shared_ptr<Util::GameObject> obj = std::make_shared<Util::GameObject>();
            std::shared_ptr<Util::Image> image = this->atlasLoader->getImage(getTile(x, y).getSpriteId());
            obj->SetDrawable(image);
            mapRoot.AddChild(obj);
            tileObjects.emplace_back(obj);
        }
    }
    // 套用初始位置
    updateTransforms();
}

// 統一更新所有地圖方塊與塔物件的位置及縮放
void GridMap::updateTransforms() {
    const float cellW = baseCellWidth * currentScale;
    const float cellH = baseCellHeight * currentScale;
    const float startX = -(mapWidth * cellW) * 0.5F + cellW * 0.5F + cameraX;
    const float startY = -(mapHeight * cellH) * 0.5F + cellH * 0.5F + cameraY;

    int i = 0;
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            tileObjects[i]->m_Transform.scale = {currentScale, currentScale};
            tileObjects[i]->m_Transform.translation = {
                startX + x * cellW,
                startY + y * cellH
            };
            i++;
        }
    }

    for (auto& tv : towerVisuals) {
        tv.obj->m_Transform.scale = {currentScale, currentScale};
        tv.obj->m_Transform.translation = {
            startX + tv.gridX * cellW,
            startY + tv.gridY * cellH
        };
    }
}

std::string GridMap::getMapName() const {
    return mapName;
}

std::string GridMap::getMapDescription() const {
    return mapDescription;
}

std::string GridMap::getMapDifficulty() const {
    return mapDifficulty;
}

Tile GridMap::getTile(int x, int y) const {
    if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) {
        throw std::out_of_range("座標超出地圖範圍");
    }
    const int row = mapHeight - 1 - y; // world y -> 檔案 row（CSV 是由上往下）
    return tilesArray[row * mapWidth + x];
}

void GridMap::moveCamera(float dx, float dy) {
    cameraX += dx;
    cameraY += dy;
    updateTransforms();
}

void GridMap::zoomCamera(float zoomDelta) {
    // 使用乘法來縮放，這會讓縮放感更自然 (例如每次放大 10% 或縮小 10%)
    // zoomDelta > 0 時放大 (例如 1.1倍)，zoomDelta < 0 時縮小 (例如 0.9倍)
    float zoomFactor = (zoomDelta > 0.0F) ? 1.05F : 0.95F; 
    
    currentScale *= zoomFactor;

    // 限制縮放範圍，避免縮得太小或放得太大
    if (currentScale < 0.1F) currentScale = 0.1F; 
    if (currentScale > 3.0F) currentScale = 3.0F; 

    updateTransforms();
}

// 將畫面上的滑鼠座標轉換回格狀座標 (x, y)
std::pair<int, int> GridMap::ScreenToGrid(float screenX, float screenY) const {
    const float cellW = baseCellWidth * currentScale;
    const float cellH = baseCellHeight * currentScale;
    const float startX = -(mapWidth * cellW) * 0.5F + cellW * 0.5F + cameraX;
    const float startY = -(mapHeight * cellH) * 0.5F + cellH * 0.5F + cameraY;

    // 四捨五入找出最近的格子中心
    int gridX = static_cast<int>(std::floor((screenX - startX + cellW * 0.5F) / cellW));
    int gridY = static_cast<int>(std::floor((screenY - startY + cellH * 0.5F) / cellH));
    return {gridX, gridY};
}

// 建立塔的視覺物件，使其隨地圖一起縮放與移動
// 參數型別改為 std::string_view
void GridMap::addTowerVisual(int gridX, int gridY, std::string_view spriteId) {
    auto obj = std::make_shared<Util::GameObject>();
    obj->SetDrawable(atlasLoader->getImage(spriteId));

    // 修復 "Member is inaccessible" 錯誤：改用 SetZIndex 設值
    obj->SetZIndex(1);

    mapRoot.AddChild(obj);
    towerVisuals.push_back({gridX, gridY, obj});
    updateTransforms();
}

int GridMap::getMapWidth() const {
    return mapWidth;
}

int GridMap::getMapHeight() const {
    return mapHeight;
}

bool GridMap::canBuildTower(int x, int y) const {
    return getTile(x, y).getType() == Tile::Type::Platform;
}

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

void GridMap::displayMap() {
    mapRoot.Update();
}

#include "map/GridMap.hpp"
#include "Core/Context.hpp"

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
    int heightCounter = 0;
    int widthCounter = -1;

    // -------------------- 讀取地圖資料 --------------------
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (isFirstLine) { mapName = line; isFirstLine = false; continue; }
        if (isSecondLine) { mapDescription = line; isSecondLine = false; continue; }
        if (isThirdLine) { mapDifficulty = line; isThirdLine = false; continue; }

        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string tileType;
        int currentRowWidth = 0;
        while (std::getline(iss, tileType, ',')) {
            if (!tileType.empty() && tileType.back() == '\r') {
                tileType.pop_back();
            }
            tilesArray.emplace_back(tileType);
            currentRowWidth++;
        }

        if (widthCounter == -1) {
            widthCounter = currentRowWidth;
        } else if (currentRowWidth != widthCounter) {
            throw std::runtime_error("地圖每一行欄數不一致");
        }
        heightCounter++;
    }

    if (heightCounter == 0) throw std::runtime_error("地圖文件中沒有地圖數據");
    mapHeight = heightCounter;
    mapWidth = widthCounter;

    // -------------------- 開始繪製地圖 --------------------
    tileObjects.reserve(tilesArray.size());

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
    const glm::vec2 firstSize = firstImage->GetSize();

    baseCellWidth = firstSize.x * kMapScale;
    baseCellHeight = firstSize.y * kMapScale;
    m_CellW = baseCellWidth;
    m_CellH = baseCellHeight;

    m_StartX = -(static_cast<float>(mapWidth) * m_CellW) * 0.5F + m_CellW * 0.5F;
    m_StartY = -(static_cast<float>(mapHeight) * m_CellH) * 0.5F + m_CellH * 0.5F;

    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            const Tile& tile = getTile(x, y);
            const Tile::Type tileType = tile.getType();
            if (tileType == Tile::Type::Empty) continue;

            if (tileType == Tile::Type::Spawn || tileType == Tile::Type::Goal) {
                std::string roadSpriteId = "tile-type-road-";
                static constexpr std::array<std::pair<int, int>, 4> kFourDirs = {{
                    {0, 1}, {1, 0}, {0, -1}, {-1, 0}
                }};
                for (const auto &[dx, dy] : kFourDirs) {
                    const int nx = x + dx;
                    const int ny = y + dy;
                    const bool inBounds = nx >= 0 && nx < mapWidth && ny >= 0 && ny < mapHeight;
                    const bool isRoad = inBounds && getTile(nx, ny).getType() == Tile::Type::Road;
                    roadSpriteId += isRoad ? "o" : "x";
                }
                std::shared_ptr<Util::GameObject> baseObj = std::make_shared<Util::GameObject>();
                std::shared_ptr<Util::Image> roadImage = atlasLoader.getImage(roadSpriteId);
                baseObj->SetDrawable(roadImage);
                baseObj->SetZIndex(kTileZIndex - 1.0F);
                mapRoot.AddChild(baseObj);
                tileObjects.push_back({x, y, baseObj});
            }

            std::shared_ptr<Util::GameObject> obj = std::make_shared<Util::GameObject>();
            std::shared_ptr<Util::Image> image = atlasLoader.getImage(tile.getSpriteId());
            obj->SetDrawable(image);
            obj->SetZIndex(kTileZIndex);
            mapRoot.AddChild(obj);
            tileObjects.push_back({x, y, obj});
        }
    }

    updateTransforms();
}

// -------------------- 相機與畫面更新 --------------------
void GridMap::updateTransforms() {
    m_CellW = baseCellWidth * currentScale;
    m_CellH = baseCellHeight * currentScale;

    // 計算置中座標，並套用相機平移
    const float startX = -(static_cast<float>(mapWidth) * m_CellW) * 0.5F + m_CellW * 0.5F + m_CameraOffsetX;
    const float startY = -(static_cast<float>(mapHeight) * m_CellH) * 0.5F + m_CellH * 0.5F + m_CameraOffsetY;

    // 供世界座標轉換用的基準也需同步更新
    m_StartX = -(static_cast<float>(mapWidth) * m_CellW) * 0.5F + m_CellW * 0.5F;
    m_StartY = -(static_cast<float>(mapHeight) * m_CellH) * 0.5F + m_CellH * 0.5F;

    const auto context = Core::Context::GetInstance();
    const float halfW = static_cast<float>(context->GetWindowWidth()) * 0.5F;
    const float halfH = static_cast<float>(context->GetWindowHeight()) * 0.5F;

    for (const TileVisual& tv : tileObjects) {
        tv.obj->m_Transform.scale = {kMapScale * currentScale, kMapScale * currentScale};
        const glm::vec2 pos = {
            startX + static_cast<float>(tv.gridX) * m_CellW,
            startY + static_cast<float>(tv.gridY) * m_CellH
        };
        tv.obj->m_Transform.translation = pos;

        // Frustum culling: check if tile is inside the window boundary (with 1.5 cell margin to prevent popping)
        const float marginX = m_CellW * 1.5F;
        const float marginY = m_CellH * 1.5F;
        const bool visible = (pos.x + marginX >= -halfW) && (pos.x - marginX <= halfW) &&
                             (pos.y + marginY >= -halfH) && (pos.y - marginY <= halfH);
        tv.obj->SetVisible(visible);
    }
}

// -------------------- 地圖描述資訊 --------------------
std::string_view GridMap::getMapName() const { return mapName; }
std::string_view GridMap::getMapDescription() const { return mapDescription; }
std::string_view GridMap::getMapDifficulty() const { return mapDifficulty; }

const Tile& GridMap::getTile(int x, int y) const {
    if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) {
        throw std::out_of_range("座標超出地圖範圍");
    }
    const int row = mapHeight - 1 - y;
    return tilesArray[row * mapWidth + x];
}

int GridMap::getMapWidth() const { return mapWidth; }
int GridMap::getMapHeight() const { return mapHeight; }

// -------------------- 規則判斷 --------------------
bool GridMap::canBuildTower(int x, int y) const {
    if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) return false;
    return getTile(x, y).getType() == Tile::Type::Platform;
}

bool GridMap::canWalk(int x, int y) const {
    if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) return false;
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

// -------------------- 座標轉換 --------------------
std::optional<std::pair<int, int>> GridMap::worldToGrid(const glm::vec2& worldPos) const {
    if (m_CellW <= 0.0F || m_CellH <= 0.0F) return std::nullopt;

    const float leftBound = (m_StartX + m_CameraOffsetX) - (m_CellW * 0.5F);
    const float bottomBound = (m_StartY + m_CameraOffsetY) - (m_CellH * 0.5F);
    const int gridX = static_cast<int>(std::floor((worldPos.x - leftBound) / m_CellW));
    const int gridY = static_cast<int>(std::floor((worldPos.y - bottomBound) / m_CellH));

    if (gridX < 0 || gridX >= mapWidth || gridY < 0 || gridY >= mapHeight) {
        return std::nullopt;
    }
    return std::pair<int, int>{gridX, gridY};
}

std::optional<glm::vec2> GridMap::gridToWorld(int gridX, int gridY) const {
    return gridToWorld(static_cast<float>(gridX), static_cast<float>(gridY));
}

std::optional<glm::vec2> GridMap::gridToWorld(float gridX, float gridY) const {
    if (gridX < 0.0F || gridX >= static_cast<float>(mapWidth) || gridY < 0.0F || gridY >= static_cast<float>(mapHeight)) {
        return std::nullopt;
    }

    return glm::vec2{
        m_StartX + gridX * m_CellW + m_CameraOffsetX,
        m_StartY + gridY * m_CellH + m_CameraOffsetY
    };
}

// -------------------- 鏡頭控制 --------------------
void GridMap::moveCamera(float dx, float dy) {
    m_CameraOffsetX += dx;
    m_CameraOffsetY += dy;
    updateTransforms();
}

void GridMap::zoomCamera(float zoomDelta) {
    float zoomFactor = (zoomDelta > 0.0F) ? 1.05F : 0.95F;
    currentScale *= zoomFactor;
    if (currentScale < 0.1F) currentScale = 0.1F;
    if (currentScale > 3.0F) currentScale = 3.0F;
    updateTransforms();
}

void GridMap::displayMap() {
    mapRoot.Update();
}
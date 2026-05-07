#pragma once

#include "Tile.hpp"
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "utils/AtlasLoader.hpp"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class GridMap {
public:
    // -------------------- 建構與基本資訊 --------------------
    GridMap(std::string_view MAP_FILE_PATH, AtlasLoader& atlas);
    std::string_view getMapName() const;
    std::string_view getMapDescription() const;
    std::string_view getMapDifficulty() const;

    // -------------------- 地圖查詢 --------------------
    const Tile& getTile(int x, int y) const;
    int getMapWidth() const;
    int getMapHeight() const;

    // -------------------- 規則判斷 --------------------
    bool canBuildTower(int x, int y) const;
    bool canWalk(int x, int y) const;

    // -------------------- 起終點查詢 --------------------
    std::vector<std::pair<int, int>> getSpawnGridPoints() const;
    std::optional<std::pair<int, int>> getGoalGridPoint() const;

    // -------------------- 座標轉換 --------------------
    std::optional<std::pair<int, int>> worldToGrid(const glm::vec2& worldPos) const;
    std::optional<glm::vec2> gridToWorld(int gridX, int gridY) const;
    std::optional<glm::vec2> gridToWorld(float gridX, float gridY) const;

    // -------------------- 顯示與鏡頭 --------------------
    void moveCamera(float dx, float dy);
    void zoomCamera(float zoomDelta);
    void displayMap();

    float getOffsetX() const { return m_CameraOffsetX; }
    float getOffsetY() const { return m_CameraOffsetY; }
    float getCurrentScale() const { return currentScale; }

private:
    void updateTransforms();

private:
    // 內部結構：用來儲存渲染物件與其對應的格狀座標
    struct TileVisual {
        int gridX;
        int gridY;
        std::shared_ptr<Util::GameObject> obj;
    };

    // -------------------- 地圖來源與描述 --------------------
    std::string MAP_FILE_PATH;
    std::string mapName;
    std::string mapDescription;
    std::string mapDifficulty;

    // -------------------- 地圖格子資料 --------------------
    int mapWidth;
    int mapHeight;
    std::vector<Tile> tilesArray;

    // -------------------- 縮放與座標快取 --------------------
    float kMapScale = 0.45F;
    float currentScale = 1.0F;
    float baseCellWidth = 0.0F;
    float baseCellHeight = 0.0F;
    float m_CellW = 0.0F;
    float m_CellH = 0.0F;
    float m_StartX = 0.0F;
    float m_StartY = 0.0F;

    // -------------------- 相機偏移快取 --------------------
    float m_CameraOffsetX = 0.0F;
    float m_CameraOffsetY = 0.0F;

    // -------------------- 渲染資料 --------------------
    static constexpr float kTileZIndex = 0.0F;
    AtlasLoader& atlasLoader;
    Util::Renderer mapRoot;
    std::vector<TileVisual> tileObjects;
};
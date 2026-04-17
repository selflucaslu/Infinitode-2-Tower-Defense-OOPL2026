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
    GridMap(std::string_view MAP_FILE_PATH, std::shared_ptr<AtlasLoader> atlas);
    [[nodiscard]] std::string getMapName() const;
    [[nodiscard]] std::string getMapDescription() const;
    [[nodiscard]] std::string getMapDifficulty() const;
    [[nodiscard]] Tile getTile(int x, int y) const;
    [[nodiscard]] int getMapWidth() const;
    [[nodiscard]] int getMapHeight() const;
    [[nodiscard]] bool canBuildTower(int x, int y) const;
    [[nodiscard]] std::vector<std::pair<int, int>> getSpawnGridPoints() const;
    [[nodiscard]] std::optional<std::pair<int, int>> getGoalGridPoint() const;

    // --- 視角與互動 ---
    void moveCamera(float dx, float dy);
    void zoomCamera(float zoomDelta);
    [[nodiscard]] std::pair<int, int> ScreenToGrid(float screenX, float screenY) const;

    // 將 std::string 改為 std::string_view 避免複製
    void addTowerVisual(int gridX, int gridY, std::string_view spriteId);

    void displayMap();

private:
    void updateTransforms(); // 統一更新所有物件的 Transform (位置與縮放)

private:
    std::string MAP_FILE_PATH;
    std::string mapName;
    std::string mapDescription;
    std::string mapDifficulty;
    int mapWidth;
    int mapHeight;
    std::vector<Tile> tilesArray;
    std::shared_ptr<AtlasLoader> atlasLoader;
    Util::Renderer mapRoot;
    std::vector<std::shared_ptr<Util::GameObject>> tileObjects;

    // --- 新增用於儲存塔視覺物件與相機狀態的變數 ---
    struct TowerVisual {
        int gridX;
        int gridY;
        std::shared_ptr<Util::GameObject> obj;
    };
    std::vector<TowerVisual> towerVisuals;

    float currentScale = 0.3F;
    float cameraX = 0.0F;
    float cameraY = 0.0F;
    float baseCellWidth = 0.0F;
    float baseCellHeight = 0.0F;
};
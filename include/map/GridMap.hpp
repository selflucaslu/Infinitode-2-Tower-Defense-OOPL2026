#pragma once

#include "Tile.hpp"
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "utils/AtlasLoader.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


class GridMap {
public:
    GridMap(std::string_view MAP_FILE_PATH, std::shared_ptr<AtlasLoader> atlas);
    std::string getMapName() const;
    std::string getMapDescription() const;
    std::string getMapDifficulty() const;
    // 第一象限座標： (0,0) 在左下，x 向右、y 向上。
    Tile getTile(int x, int y) const;
    int getMapWidth() const;
    int getMapHeight() const;
    bool canBuildTower(int x, int y) const; // 判斷該位置是否可以建造塔
    std::pair<float, float> getTileCenterWorld(int x, int y) const;
    void moveCamera(float dx, float dy);
    void displayMap(); // 用於顯示地圖的函數
private:
    std::string MAP_FILE_PATH;       // 地圖文件路徑
    std::string mapName;             // 地圖名稱
    std::string mapDescription;      // 地圖描述
    std::string mapDifficulty;       // 地圖難度
    int mapWidth;                    // 地圖寬度
    int mapHeight;                   // 地圖高度
    std::vector<Tile> tilesArray;    // 用於存儲地圖上每個 tile 的一維陣列
    std::shared_ptr<AtlasLoader> atlasLoader;
    Util::Renderer mapRoot;
    std::vector<std::shared_ptr<Util::GameObject>> tileObjects;
    float cellW = 0.0F;
    float cellH = 0.0F;
    float startX = 0.0F;
    float startY = 0.0F;
};

#pragma once

#include "Tile.hpp"
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "utils/AtlasLoader.hpp"

#include <string_view>
#include <string>
#include <vector>


class GridMap {
public:
    GridMap(std::string_view MAP_FILE_PATH);
    std::string getMapName() const;
    std::string getMapDescription() const;
    std::string getMapDifficulty() const;
    // 第一象限座標： (0,0) 在左下，x 向右、y 向上。
    Tile getTile(int x, int y) const;
    void displayMap(); // 用於顯示地圖的函數
private:
    std::string_view MAP_FILE_PATH;  // 地圖文件路徑
    std::string mapName;             // 地圖名稱
    std::string mapDescription;      // 地圖描述
    std::string mapDifficulty;       // 地圖難度
    int mapWidth;                    // 地圖寬度
    int mapHeight;                   // 地圖高度
    std::vector<Tile> tilesArray;    // 用於存儲地圖上每個 tile 的一維陣列
    AtlasLoader atlasLoader;
    Util::Renderer mapRoot;
    std::vector<std::shared_ptr<Util::GameObject>> tileObjects;
};

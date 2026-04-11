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
    GridMap(std::string_view MAP_FILE_PATH, std::shared_ptr<AtlasLoader> atlas); // 載入地圖與建立顯示物件
    std::string getMapName() const; // 取得地圖名稱
    std::string getMapDescription() const; // 取得地圖描述
    std::string getMapDifficulty() const; // 取得地圖難度
    // 第一象限座標： (0,0) 在左下，x 向右、y 向上。
    Tile getTile(int x, int y) const;
    int getMapWidth() const;
    int getMapHeight() const;
    bool canBuildTower(int x, int y) const; // 判斷該位置是否可以建造塔
    std::vector<std::pair<int, int>> getSpawnGridPoints() const; // 取得所有起點格座標
    std::optional<std::pair<int, int>> getGoalGridPoint() const; // 取得終點格座標（沒有則回傳 nullopt）
    void moveCamera(float dx, float dy); // 平移地圖視角
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
};

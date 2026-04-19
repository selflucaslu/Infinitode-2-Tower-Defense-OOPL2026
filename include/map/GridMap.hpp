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
    GridMap(std::string_view MAP_FILE_PATH, AtlasLoader& atlas); // 載入地圖與建立顯示物件
    std::string_view getMapName() const; // 取得地圖名稱
    std::string_view getMapDescription() const; // 取得地圖描述
    std::string_view getMapDifficulty() const; // 取得地圖難度

    // -------------------- 地圖查詢 --------------------
    // 第一象限座標： (0,0) 在左下，x 向右、y 向上。
    const Tile& getTile(int x, int y) const;
    int getMapWidth() const;
    int getMapHeight() const;

    // -------------------- 規則判斷 --------------------
    // 判斷該位置是否可以建造塔
    bool canBuildTower(int x, int y) const;
    // 判斷該位置是否可行走（敵人路徑）
    bool canWalk(int x, int y) const;

    // -------------------- 起終點查詢 --------------------
    // 取得所有起點格座標（第一象限座標）
    std::vector<std::pair<int, int>> getSpawnGridPoints() const;
    // 取得終點格座標（沒有則回傳 nullopt）
    std::optional<std::pair<int, int>> getGoalGridPoint() const;

    // -------------------- 滑鼠點擊轉換世界座標 --------------------
    std::optional<std::pair<int, int>> worldToGrid(const glm::vec2& worldPos) const;

    // -------------------- 顯示與鏡頭 --------------------
    void moveCamera(float dx, float dy); // 平移地圖視角
    void displayMap(); // 用於顯示地圖的函數
private:
    // -------------------- 地圖來源與描述 --------------------
    std::string MAP_FILE_PATH;       // 地圖文件路徑
    std::string mapName;             // 地圖名稱
    std::string mapDescription;      // 地圖描述
    std::string mapDifficulty;       // 地圖難度

    // -------------------- 地圖格子資料 --------------------
    int mapWidth;                    // 地圖寬度
    int mapHeight;                   // 地圖高度
    std::vector<Tile> tilesArray;    // 用於存儲地圖上每個 tile 的一維陣列

    // -------------------- 格子轉世界座標快取 --------------------
    float kMapScale = 0.45F; // 地圖整體縮放倍率
    float m_CellW = 0.0F; // 單格世界寬度（縮放後）
    float m_CellH = 0.0F; // 單格世界高度（縮放後）
    float m_StartX = 0.0F; // 地圖左下格中心世界座標 x（置中後）
    float m_StartY = 0.0F; // 地圖左下格中心世界座標 y（置中後）

    // -------------------- 相機偏移快取 --------------------
    float m_CameraOffsetX = 0.0F; // 敵人整體視角偏移 x（與地圖同步）
    float m_CameraOffsetY = 0.0F; // 敵人整體視角偏移 y（與地圖同步）

    // -------------------- 渲染資料 --------------------
    static constexpr float kTileZIndex = 0.0F; // 所有地圖格子的統一 Z 軸層級
    AtlasLoader& atlasLoader; // 圖集載入器參考（不擁有）
    Util::Renderer mapRoot; // 地圖渲染根節點
    std::vector<std::shared_ptr<Util::GameObject>> tileObjects; // 與 tilesArray 對應的渲染物件
};

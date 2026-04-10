#pragma once

#include "map/GridMap.hpp"
#include "utils/AtlasLoader.hpp"

#include <memory>
#include <string_view>

class GameSession {
public:
    // 建立單局資料（地圖、基地血量、波次）。
    GameSession(
        std::string_view mapFilePath,
        int initialBaseHp = 20,
        int initialWave = 1
    );

    GridMap& getMap(); // 可修改地圖實體
    const GridMap& getMap() const; // 唯讀地圖實體

    int getBaseHp() const; // 取得基地血量
    void setBaseHp(int newBaseHp); // 設定基地血量
    void applyBaseDamage(int damage); // 扣除基地血量
    bool isBaseAlive() const; // 基地是否仍存活

    int getWave() const; // 取得目前波次
    void setWave(int newWave); // 設定目前波次
    void nextWave(); // 進入下一波

private:
    std::shared_ptr<AtlasLoader> atlasLoader; // 圖集載入器（共用資源）
    std::unique_ptr<GridMap> map; // 本局地圖實體
    int baseHp; // 基地血量
    int wave; // 目前波次
};

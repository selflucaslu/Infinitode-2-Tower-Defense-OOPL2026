#pragma once

#include "map/GridMap.hpp"
#include "utils/AtlasLoader.hpp"

#include <memory>
#include <string_view>
#include "tower/TowerManager.hpp"

class GameSession {
public:
    // 建立單局資料（地圖、基地血量、波次）。
    GameSession(
        std::string_view mapFilePath,
        int initialBaseHp = 20,
        int initialWave = 1
    );

    [[nodiscard]] GridMap& getMap();
    [[nodiscard]] const GridMap& getMap() const;

    [[nodiscard]] int getBaseHp() const;
    void setBaseHp(int newBaseHp);
    void applyBaseDamage(int damage);
    [[nodiscard]] bool isBaseAlive() const;

    [[nodiscard]] int getWave() const;
    void setWave(int newWave);
    void nextWave();

    TowerManager& getTowerManager(); // 新增存取器

private:
    std::shared_ptr<AtlasLoader> atlasLoader; // 圖集載入器（共用資源）
    std::unique_ptr<GridMap> map; // 本局地圖實體
    int baseHp; // 基地血量
    int wave; // 目前波次
    std::unique_ptr<TowerManager> towerManager; // 新增成員
};

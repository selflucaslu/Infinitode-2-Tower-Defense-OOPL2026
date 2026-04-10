#pragma once

#include "map/GridMap.hpp"
#include "utils/AtlasLoader.hpp"

#include <memory>
#include <string_view>
#include "tower/TowerManager.hpp"

class EnemyManager;

class GameSession {
public:
    // 建立單局資料（地圖、基地血量、波次）。
    GameSession(
        std::string_view mapFilePath,
        int initialBaseHp = 20,
        int initialWave = 1
    );
    ~GameSession();

    GridMap& getMap(); // 可修改地圖實體
    const GridMap& getMap() const; // 唯讀地圖實體
    std::shared_ptr<AtlasLoader> getAtlasLoader() const; // 取得共用圖集載入器
    EnemyManager& getEnemyManager(); // 取得敵人管理器
    const EnemyManager& getEnemyManager() const; // 取得唯讀敵人管理器

    void update(float deltaTimeSec); // 推進本局更新（含產怪週期）
    void moveCamera(float dx, float dy); // 平移本局地圖與敵人視角
    void display(); // 顯示本局地圖與敵人

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
    std::unique_ptr<EnemyManager> enemyManager; // 本局敵人管理器（含產怪週期）
    int baseHp; // 基地血量
    int wave; // 目前波次
    std::unique_ptr<TowerManager> towerManager; // 新增成員
};

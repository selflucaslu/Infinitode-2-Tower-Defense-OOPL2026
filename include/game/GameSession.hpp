#pragma once

#include "enemy/EnemyManager.hpp"
#include "enemy/EnemyTypeConfig.hpp"
#include "map/GridMap.hpp"
#include "tower/TowerManager.hpp"
#include "utils/AtlasLoader.hpp"
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"
#include "game/LevelConfig.hpp"

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

class GameSession {
public:
    // -------------------- 建立與生命週期 --------------------
    GameSession(int levelNumber = 1);

    // -------------------- 地圖與敵人管理 --------------------
    GridMap& getMap();
    const GridMap& getMap() const;
    EnemyManager& getEnemyManager();
    const EnemyManager& getEnemyManager() const;

    // -------------------- 塔管理 --------------------
    TowerManager& getTowerManager();
    const TowerManager& getTowerManager() const;
    bool placeTower(int gridX, int gridY, std::string_view spriteId = "tower-basic");

    // -------------------- 基地狀態 --------------------
    int getBaseHp() const;
    int getGold() const;
    void setBaseHp(int newBaseHp);
    void applyBaseDamage(int damage);
    void addGold(int amount);
    bool isBaseAlive() const;

    // -------------------- 波次狀態 --------------------
    int getWave() const;
    void setWave(int newWave);
    void nextWave();

    // -------------------- 每幀流程 --------------------
    void update(float deltaTime);
    void display();

    // 修正 Clang-Tidy：加上 const
    void moveCamera(float dx, float dy) const;
    void zoomCamera(float zoomDelta) const;

    // -------------------- 遊戲流程控制 --------------------
    void initSession();
    void startSession();
    void pauseSession();
    void dispatchEnemiesByTimer();

    // -------------------- 測試入口 --------------------
    // 修正 Clang-Tidy：加上 const
    void spawnDebugEnemy(
        EnemyTypeId enemyTypeId,
        const std::vector<int>& spawnPointIndices = {}
    ) const;

private:
    void updateTowerDisplay();
    void updateProjectileDisplay();
    void updateHudDisplay();

private:
    // -------------------- 資源與核心物件 --------------------
    std::unique_ptr<AtlasLoader> atlasLoader;
    std::unique_ptr<GridMap> map;
    std::unique_ptr<EnemyManager> enemyManager;
    std::unique_ptr<TowerManager> towerManager;
    Util::Renderer towerRoot;
    std::vector<std::shared_ptr<Util::GameObject>> towerBaseObjects;
    std::vector<std::shared_ptr<Util::GameObject>> towerWeaponObjects;
    Util::Renderer projectileRoot;
    std::vector<std::shared_ptr<Util::GameObject>> projectileObjects;
    Util::Renderer hudRoot;
    std::shared_ptr<Util::GameObject> towerHpIconObject;
    std::shared_ptr<Util::GameObject> towerHpTextObject;
    std::shared_ptr<Util::Text> towerHpText;
    std::shared_ptr<Util::GameObject> goldIconObject;
    std::shared_ptr<Util::GameObject> goldTextObject;
    std::shared_ptr<Util::Text> goldText;
    std::shared_ptr<Util::GameObject> waveIconObject;
    std::shared_ptr<Util::GameObject> waveTextObject;
    std::shared_ptr<Util::Text> waveText;
    static constexpr float kTowerScale = 0.45F;
    static constexpr float kTowerBaseZIndex = 1.5F;
    static constexpr float kTowerWeaponZIndex = 1.6F;
    static constexpr float kProjectileScale = 0.22F;
    static constexpr float kProjectileZIndex = 2.3F;
    static constexpr const char* kHudFontPath = "PTSD/assets/fonts/Inter.ttf";
    static constexpr int kHudFontSize = 24;
    static constexpr float kHudZIndex = 4.0F;
    static constexpr float kHudPadding = 16.0F;
    static constexpr float kHudIconScale = 0.34F;
    static constexpr float kHudGap = 10.0F;
    static constexpr int kTowerBuildCost = 40;

    // -------------------- 遊戲狀態 --------------------
    bool isSessionActive;
    float timer;
    float waveTimer;
    float groupTimer;
    int initBaseHp;
    int initGold;
    int baseHp;
    int gold;
    int waveCount;
    int groupIndex;
    int groupSpawned;
    std::vector<WaveConfig> spawnSchedule;
};
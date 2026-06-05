#pragma once

#include "enemy/EnemyManager.hpp"
#include "enemy/EnemyTypeConfig.hpp"
#include "map/GridMap.hpp"
#include "tower/TowerManager.hpp"
#include "tower/TowerDef.hpp"
#include "utils/AtlasLoader.hpp"
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"
#include "game/LevelConfig.hpp"
#include "utils/FpsOverlay.hpp"
#include "utils/TowerSelectionPanel.hpp"

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

class GameSession {
public:
    // -------------------- 建立與生命週期 --------------------
    GameSession(int levelNumber = 4);

    // -------------------- 地圖與敵人管理 --------------------
    GridMap& getMap();
    const GridMap& getMap() const;
    EnemyManager& getEnemyManager();
    const EnemyManager& getEnemyManager() const;

    // -------------------- 塔管理 --------------------
    TowerManager& getTowerManager();
    const TowerManager& getTowerManager() const;
    bool placeTower(int gridX, int gridY, TowerId towerId = TowerId::Basic);
    bool sellTower(int gridX, int gridY);

    // -------------------- 選塔面板 --------------------
    void setSelectedTower(TowerId id);             // 更新選塔面板高亮
    TowerId getSelectedTower() const;              // 取得目前選中的塔種類
    // 根據螢幕座標（OpenGL 中心座標）判斷是否點擊到選塔格子
    std::optional<TowerId> hitTestSelectionPanel(float screenX, float screenY) const;

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
    int getLevelNumber() const;
    bool isLevelCompleted() const;

    // -------------------- 每幀流程 --------------------
    void update(float deltaTime);
    void update(float deltaTime, float rawDeltaTime);
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
    std::unique_ptr<TowerSelectionPanel> m_SelectionPanel; // 選塔面板（右下角格子）
    std::unique_ptr<FpsOverlay> m_FpsOverlay; // FPS 顯示工具（只在遊戲畫面顯示）
    static constexpr float kTowerScale = 0.45F;
    static constexpr float kTowerBaseZIndex = 1.5F;
    static constexpr float kTowerWeaponZIndex = 1.6F;
    static constexpr float kProjectileScale = 0.22F;
    static constexpr float kProjectileZIndex = 2.3F;
    static constexpr const char* kHudFontPath = "PTSD/assets/fonts/Inter.ttf";
    static constexpr int kHudFontSize = 28;
    static constexpr float kHudZIndex = 4.0F;
    static constexpr float kHudPadding = 16.0F;
    static constexpr float kHudIconScale = 0.52F;
    static constexpr float kHudGap = 8.0F;
    static constexpr int kTowerBuildCost = 40; // fallback only — actual cost from TowerDef

    // -------------------- 遊戲狀態 --------------------
    bool isSessionActive;
    float timer;
    float waveTimer;
    float groupTimer;
    int initBaseHp;
    int initGold;
    int levelNumber;
    int baseHp;
    int gold;
    int waveCount;
    int groupIndex;
    int groupSpawned;
    int loopCount;                         // 已完成的循環次數（0 = 第一輪）
    bool levelCompleted;                   // 本關已達成換到下一關的條件
    std::vector<WaveConfig> spawnSchedule; // 當前循環（已套用強化）的配置
    std::vector<WaveConfig> baseSpawnSchedule; // 第一輪的原始配置（用於重置計算）

    // -------------------- 私有方法 --------------------
    bool hasNextLevel() const;
    void beginNextLoop(); // 啟動下一循環（套用血量 ×1.2、速度 ×1.1）
};

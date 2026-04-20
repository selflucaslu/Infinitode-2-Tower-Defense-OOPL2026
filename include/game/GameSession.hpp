#pragma once

#include "enemy/EnemyManager.hpp"
#include "enemy/EnemyTypeConfig.hpp"
#include "map/GridMap.hpp"
#include "tower/TowerManager.hpp"
#include "utils/AtlasLoader.hpp"
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "game/LevelConfig.hpp"

#include <memory>
#include <string_view>
#include <utility>
#include <vector>

class GameSession {
public:
    // -------------------- 建立與生命週期 --------------------
    // 建立單局資料（地圖、基地血量、波次）。
    GameSession(int levelNumber = 1);

    // -------------------- 地圖與敵人管理 --------------------
    GridMap& getMap(); // 可修改地圖實體
    const GridMap& getMap() const; // 唯讀地圖實體
    EnemyManager& getEnemyManager(); // 可修改敵人管理器
    const EnemyManager& getEnemyManager() const; // 唯讀敵人管理器

    // -------------------- 塔管理 --------------------
    TowerManager& getTowerManager(); // 可修改塔管理器
    const TowerManager& getTowerManager() const; // 唯讀塔管理器
    bool placeTower(int gridX, int gridY, std::string_view spriteId = "tower-basic"); // 在格子上嘗試建塔

    // -------------------- 基地狀態 --------------------
    int getBaseHp() const; // 取得基地血量
    int getGold() const; // 取得目前金幣
    void setBaseHp(int newBaseHp); // 設定基地血量
    void applyBaseDamage(int damage); // 扣除基地血量
    void addGold(int amount); // 增加金幣
    bool isBaseAlive() const; // 基地是否仍存活

    // -------------------- 波次狀態 --------------------
    int getWave() const; // 取得目前波次
    void setWave(int newWave); // 設定目前波次
    void nextWave(); // 進入下一波

    // -------------------- 每幀流程 --------------------
    // 統一由 GameSession 驅動本局物件更新與顯示。
    void update(float deltaTime);
    void display();
    void moveCamera(float dx, float dy);

    // -------------------- 遊戲流程控制 --------------------
    void initSession(); // 初始化本局參數（重置 timer、基地血量、金幣、波次、敵人等）
    void startSession(); // 開始/恢復遊戲
    void pauseSession(); // 暫停遊戲
    void dispatchEnemiesByTimer(); // 根據 timer 觸發敵人生成

    // -------------------- 測試入口 --------------------
    // Debug 用生怪入口：由 App 觸發，GameSession 轉發給 EnemyManager。
    void spawnDebugEnemy(
        EnemyTypeId enemyTypeId,
        const std::vector<int>& spawnPointIndices = {}
    );

private:
    void updateTowerDisplay(); // 同步塔渲染物件與位置
    void updateProjectileDisplay(); // 同步子彈渲染物件與位置

private:
    // -------------------- 資源與核心物件 --------------------
    std::unique_ptr<AtlasLoader> atlasLoader; // 圖集載入器（本局唯一擁有，集中資源管理）
    std::unique_ptr<GridMap> map; // 本局地圖實體
    std::unique_ptr<EnemyManager> enemyManager; // 本局敵人管理器（使用 map 與 atlasLoader 的參考）
    std::unique_ptr<TowerManager> towerManager; // 本局塔管理器（使用 map 參考）
    Util::Renderer towerRoot; // 塔渲染根節點
    std::vector<std::shared_ptr<Util::GameObject>> towerObjects; // 與塔容器對應的渲染物件
    Util::Renderer projectileRoot; // 子彈渲染根節點
    std::vector<std::shared_ptr<Util::GameObject>> projectileObjects; // 與子彈容器對應的渲染物件
    static constexpr float kTowerScale = 0.45F;
    static constexpr float kTowerZIndex = 1.5F;
    static constexpr float kProjectileScale = 0.22F;
    static constexpr float kProjectileZIndex = 2.3F;

    // -------------------- 遊戲狀態 --------------------
    bool isSessionActive; // 本局是否在進行中
    float timer; // 本局經過時間（秒）
    float waveTimer; // 本波經過時間（秒）
    float groupTimer; // 目前 group 內部計時（秒）
    int initBaseHp; // 初始基地血量
    int initGold; // 初始金幣
    int baseHp; // 基地血量
    int gold; // 金幣
    int waveCount; // 目前波次
    int groupIndex; // 目前 group 索引（每波內依序處理）
    int groupSpawned; // 目前 group 已生成數量
    std::vector<WaveConfig> spawnSchedule; 
};

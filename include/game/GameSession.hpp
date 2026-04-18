#pragma once

#include "enemy/EnemyManager.hpp"
#include "enemy/EnemyTypeConfig.hpp"
#include "map/GridMap.hpp"
#include "utils/AtlasLoader.hpp"

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

class GameSession {
public:
    // -------------------- 建立與生命週期 --------------------
    // 建立單局資料（地圖、基地血量、波次）。
    GameSession(
        std::string_view mapFilePath,
        int initialBaseHp = 20,
        int initialWave = 1
    );

    // -------------------- 地圖與敵人管理 --------------------
    GridMap& getMap(); // 可修改地圖實體
    const GridMap& getMap() const; // 唯讀地圖實體
    EnemyManager& getEnemyManager(); // 可修改敵人管理器
    const EnemyManager& getEnemyManager() const; // 唯讀敵人管理器

    // -------------------- 基地狀態 --------------------
    int getBaseHp() const; // 取得基地血量
    void setBaseHp(int newBaseHp); // 設定基地血量
    void applyBaseDamage(int damage); // 扣除基地血量
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

    // -------------------- 測試入口 --------------------
    // Debug 用生怪入口：由 App 觸發，GameSession 轉發給 EnemyManager。
    void spawnDebugEnemy(
        EnemyTypeId enemyTypeId,
        const std::vector<std::size_t>& spawnPointIndices = {}
    );

private:
    // -------------------- 資源與核心物件 --------------------
    std::unique_ptr<AtlasLoader> atlasLoader; // 圖集載入器（本局唯一擁有，集中資源管理）
    std::unique_ptr<GridMap> map; // 本局地圖實體
    std::unique_ptr<EnemyManager> enemyManager; // 本局敵人管理器（使用 map 與 atlasLoader 的參考）

    // -------------------- 遊戲狀態 --------------------
    int baseHp; // 基地血量
    int wave; // 目前波次
};

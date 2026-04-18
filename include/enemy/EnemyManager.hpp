#pragma once

#include "enemy/Enemy.hpp"
#include "map/GridMap.hpp"
#include "utils/AtlasLoader.hpp"
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class EnemyManager {
public:
    float kMapScale = 0.45F; // 敵人渲染縮放倍率（需與地圖視覺比例一致）

    // -------------------- 建構 --------------------
    // 綁定地圖與圖集載入器，並初始化固定路徑與敵人渲染基準參數。
    EnemyManager(const GridMap& map, AtlasLoader& atlasLoader);

    // -------------------- 生怪 --------------------
    // 依指定起點索引批量生怪（例如 {0}, {0,2}, {0,1,2}）。
    // 若傳入空陣列，代表所有起點都生一隻。
    void spawnEnemiesAt(
        const std::vector<std::size_t>& spawnPointIndices,
        float speed = 1.0F,
        Enemy::MoveType moveType = Enemy::MoveType::Ground,
        int maxHealth = 40,
        int damage = 5,
        std::string_view spriteId = "enemy-type-regular"
    );

    // -------------------- 每幀更新 --------------------
    // 更新敵人狀態（不做複雜尋路，沿固定路徑移動）。
    // 同步維護敵人的渲染物件（貼圖與座標）。
    void update(float deltaTime);
    // 統一繪製敵人。
    void display();
    // 平移敵人視角（行為對齊 GridMap::moveCamera）。
    void moveCamera(float dx, float dy);

    // -------------------- 收集與清理 --------------------
    // 收集到終點敵人的總傷害值（供外部扣 Base HP）。
    int collectReachedGoalDamage();
    // 移除已死亡敵人。
    void removeDeadEnemies();
    // 移除已死亡或已到終點敵人（常用於每幀收尾清理）。
    void removeDeadAndReached();

    // -------------------- 容器存取 --------------------
    // 取得敵人容器（供外部系統，如塔攻擊或除錯資訊）。
    std::vector<Enemy>& getEnemies();
    // 取得唯讀敵人容器。
    const std::vector<Enemy>& getEnemies() const;

    void buildPathsFromMap(); // 從地圖資訊建立固定路徑與起點資料
private:
    // -------------------- 外部依賴（不擁有） --------------------
    const GridMap& m_Map; // 地圖參考（用於路徑資訊）
    AtlasLoader& m_AtlasLoader; // 圖集載入器參考（不擁有）

    // -------------------- 渲染資料 --------------------
    Util::Renderer m_EnemyRoot; // 敵人物件渲染根節點
    std::vector<std::shared_ptr<Util::GameObject>> m_EnemyObjects; // 敵人渲染物件（與 enemies 位置一一對應）

    // -------------------- 敵人與路徑資料 --------------------
    std::vector<Enemy> enemies; // 場上敵人容器
    std::vector<std::shared_ptr<const std::vector<std::pair<int, int>>>> fixedPathPoints; // 起點到終點的固定路徑格子列表(含所有起點開始的路徑)

    // -------------------- 格子轉世界座標快取 --------------------
    float m_MapScale = kMapScale; // EnemyManager 內部使用的縮放倍率
    float m_CellW = 0.0F; // 單格世界寬度（縮放後）
    float m_CellH = 0.0F; // 單格世界高度（縮放後）
    float m_StartX = 0.0F; // 地圖左下格中心世界座標 x（置中後）
    float m_StartY = 0.0F; // 地圖左下格中心世界座標 y（置中後）

    // -------------------- 相機偏移快取 --------------------
    float m_CameraOffsetX = 0.0F; // 敵人整體視角偏移 x（與地圖同步）
    float m_CameraOffsetY = 0.0F; // 敵人整體視角偏移 y（與地圖同步）
};

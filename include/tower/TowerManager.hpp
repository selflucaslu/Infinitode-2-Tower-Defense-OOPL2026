#pragma once

#include "enemy/Enemy.hpp"
#include "map/GridMap.hpp"
#include "tower/Tower.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class TowerManager {
public:
    struct Projectile {
        float x = 0.0F; // 子彈目前 x（格子座標）
        float y = 0.0F; // 子彈目前 y（格子座標）
        float dirX = 0.0F; // 子彈方向 x
        float dirY = 0.0F; // 子彈方向 y
        float speed = 8.0F; // 子彈速度（格/秒）
        int damage = 15; // 子彈傷害
        float lifetime = 2.0F; // 最長存活秒數
    };

    // 綁定地圖，供建塔檢查與座標範圍判斷使用。
    explicit TowerManager(const GridMap& map);

    // 建立/移除/查詢塔。
    bool placeTower(int gridX, int gridY, std::string_view spriteId = "tower-basic");
    bool removeTower(int gridX, int gridY);
    bool hasTower(int gridX, int gridY) const;
    void clear();

    // 塔自動攻擊開關與更新（攻擊最近敵人，發射追蹤子彈）。
    void setAutoAttackEnabled(bool enabled);
    bool getAutoAttackEnabled() const;
    void updateAutoAttack(float deltaTime, std::vector<Enemy>& enemies);

    // 取得單塔或全部塔。
    std::optional<std::reference_wrapper<Tower>> getTower(int gridX, int gridY);
    std::optional<std::reference_wrapper<const Tower>> getTower(int gridX, int gridY) const;
    std::vector<Tower>& getTowers();
    const std::vector<Tower>& getTowers() const;
    const std::vector<Projectile>& getProjectiles() const;

private:
    // 尋找指定塔可攻擊範圍內的最近敵人。
    std::optional<std::size_t> findNearestEnemyIndex(const Tower& tower, const std::vector<Enemy>& enemies) const;
    // 依格子座標找塔索引。
    std::optional<std::size_t> findTowerIndex(int gridX, int gridY) const;

private:
    static constexpr float kTowerFireInterval = 0.35F;
    static constexpr float kProjectileHitRadius = 0.22F;
    static constexpr float kProjectileSeekRadius = 6.0F;
    const GridMap& map; // 地圖參考（不擁有）
    std::vector<Tower> towers; // 場上塔容器
    std::vector<float> towerCooldowns; // 各塔剩餘冷卻秒數
    std::vector<Projectile> projectiles; // 場上子彈容器
    bool autoAttackEnabled; // 是否啟用自動攻擊（由 cpp 初始化）
};

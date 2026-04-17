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
    // 綁定地圖，供建塔檢查與座標範圍判斷使用。
    explicit TowerManager(const GridMap& map);

    // 建立/移除/查詢塔。
    bool placeTower(int gridX, int gridY, std::string_view spriteId = "tower-basic");
    bool removeTower(int gridX, int gridY);
    bool hasTower(int gridX, int gridY) const;

    // 塔自動攻擊開關與更新（攻擊最近敵人，直傷，不做子彈）。
    void setAutoAttackEnabled(bool enabled);
    bool getAutoAttackEnabled() const;
    void updateAutoAttack(float deltaTime, std::vector<Enemy>& enemies);

    // 取得單塔或全部塔。
    std::optional<std::reference_wrapper<Tower>> getTower(int gridX, int gridY);
    std::optional<std::reference_wrapper<const Tower>> getTower(int gridX, int gridY) const;
    std::vector<Tower>& getTowers();
    const std::vector<Tower>& getTowers() const;

private:
    // 尋找指定塔可攻擊範圍內的最近敵人。
    std::optional<std::size_t> findNearestEnemyIndex(const Tower& tower, const std::vector<Enemy>& enemies) const;
    // 依格子座標找塔索引。
    std::optional<std::size_t> findTowerIndex(int gridX, int gridY) const;

private:
    const GridMap& map; // 地圖參考（不擁有）
    std::vector<Tower> towers; // 場上塔容器
    bool autoAttackEnabled; // 是否啟用自動攻擊（由 cpp 初始化）
};

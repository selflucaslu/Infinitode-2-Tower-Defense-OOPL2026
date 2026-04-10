#pragma once

#include "enemy/Enemy.hpp"

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class EnemyManager {
public:
    // 路徑點與起點座標，使用世界座標（x, y）。
    using Point = std::pair<float, float>;

    EnemyManager();

    // 直接在指定世界座標產生單一敵人。
    void spawnEnemy(
        float startX,
        float startY,
        float speed = 1.0F,
        int maxHealth = 40,
        int damage = 5,
        std::string_view spriteId = "enemy-type-regular"
    );

    // 從第 N 個起點產生單一敵人。
    bool spawnEnemyAtSpawnPoint(
        std::size_t spawnPointIndex,
        float speed = 1.0F,
        int maxHealth = 40,
        int damage = 5,
        std::string_view spriteId = "enemy-type-regular"
    );

    // 每個起點都產生一隻敵人。
    void spawnEnemyFromAllStartPoints(
        float speed = 1.0F,
        int maxHealth = 40,
        int damage = 5,
        std::string_view spriteId = "enemy-type-regular"
    );

    // 固定路徑與起點資料設定。
    void setFixedPath(std::vector<Point> pathPoints);
    const std::vector<Point>& getFixedPath() const;
    void setSpawnPoints(std::vector<Point> spawnPoints);
    const std::vector<Point>& getSpawnPoints() const;

    // 更新敵人狀態（不做複雜尋路，沿固定路徑移動）。
    void update(float deltaTime);
    void update(float deltaTime, const std::vector<Point>& pathPoints);

    // 收集到終點敵人的總傷害值（供外部扣 Base HP）。
    int collectReachedGoalDamage();
    void removeDeadEnemies();
    void removeDeadAndReached();

    // Base HP 狀態維護。
    void setBaseHp(int newBaseHp);
    int getBaseHp() const;
    bool isBaseAlive() const;
    void applyBaseDamage(int damage);

    std::vector<Enemy>& getEnemies();
    const std::vector<Enemy>& getEnemies() const;

private:
    std::vector<Enemy> enemies; // 場上敵人容器
    std::vector<Point> fixedPathPoints; // 固定路徑點序列
    std::vector<Point> spawnPoints; // 所有起點座標
    int baseHp; // 基地血量（由 cpp 初始化）
};

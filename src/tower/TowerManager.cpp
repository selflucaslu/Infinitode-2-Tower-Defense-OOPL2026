#include "tower/TowerManager.hpp"

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <limits>
#include <optional>
#include <utility>

namespace {
float squaredDistance(float x1, float y1, float x2, float y2) {
    const float dx = x1 - x2;
    const float dy = y1 - y2;
    return dx * dx + dy * dy;
}
} // namespace

TowerManager::TowerManager(const GridMap& map)
    : map(map), autoAttackEnabled(true) {
}

bool TowerManager::placeTower(int gridX, int gridY, TowerId towerId) {
    if (gridX < 0 || gridX >= map.getMapWidth() || gridY < 0 || gridY >= map.getMapHeight()) return false;
    if (!map.canBuildTower(gridX, gridY)) return false;
    if (hasTower(gridX, gridY)) return false;

    const TowerDef& def = getTowerDef(towerId);
    towers.emplace_back(std::string(def.spriteBase), gridX, gridY, def.attackRange);
    towerIds.push_back(towerId);
    towerCooldowns.push_back(0.0F);
    return true;
}

bool TowerManager::placeTower(int gridX, int gridY, std::string_view spriteId) {
    // Legacy overload: maps known spriteIds to TowerId, else places Basic.
    if (spriteId == "tower-sniper" || spriteId == "tower-sniper-base-new") return placeTower(gridX, gridY, TowerId::Sniper);
    if (spriteId == "tower-cannon" || spriteId == "tower-cannon-base-new") return placeTower(gridX, gridY, TowerId::Cannon);
    return placeTower(gridX, gridY, TowerId::Basic);
}

bool TowerManager::removeTower(int gridX, int gridY) {
    const std::optional<std::size_t> towerIndexOpt = findTowerIndex(gridX, gridY);
    if (!towerIndexOpt.has_value()) return false;

    // 修正 Clang-Tidy：使用 auto
    auto towerIndex = static_cast<std::ptrdiff_t>(towerIndexOpt.value());
    towers.erase(towers.begin() + towerIndex);
    towerIds.erase(towerIds.begin() + towerIndex);
    towerCooldowns.erase(towerCooldowns.begin() + towerIndex);
    return true;
}

bool TowerManager::hasTower(int gridX, int gridY) const {
    return findTowerIndex(gridX, gridY).has_value();
}

void TowerManager::clear() {
    towers.clear();
    towerIds.clear();
    towerCooldowns.clear();
    projectiles.clear();
}

std::optional<std::reference_wrapper<Tower>> TowerManager::getTower(int gridX, int gridY) {
    const std::optional<std::size_t> towerIndexOpt = findTowerIndex(gridX, gridY);
    if (!towerIndexOpt.has_value()) return std::nullopt;
    return towers[towerIndexOpt.value()];
}

std::optional<std::reference_wrapper<const Tower>> TowerManager::getTower(int gridX, int gridY) const {
    const std::optional<std::size_t> towerIndexOpt = findTowerIndex(gridX, gridY);
    if (!towerIndexOpt.has_value()) return std::nullopt;
    return towers[towerIndexOpt.value()];
}

std::vector<Tower>& TowerManager::getTowers() { return towers; }
const std::vector<Tower>& TowerManager::getTowers() const { return towers; }
const std::vector<TowerManager::Projectile>& TowerManager::getProjectiles() const { return projectiles; }

std::optional<std::size_t> TowerManager::findNearestEnemyIndex(const Tower& tower, const std::vector<Enemy>& enemies) const {
    const float towerX = static_cast<float>(tower.GetGridX());
    const float towerY = static_cast<float>(tower.GetGridY());
    const float rangeSquared = tower.GetAttackRange() * tower.GetAttackRange();

    std::optional<std::size_t> nearestEnemyIndex = std::nullopt;
    float nearestDistanceSquared = std::numeric_limits<float>::max();

    for (std::size_t i = 0; i < enemies.size(); ++i) {
        const Enemy& enemy = enemies[i];
        if (!enemy.isAlive() || enemy.hasReachedGoal()) continue;

        const float enemyX = enemy.getX();
        const float enemyY = enemy.getY();
        const float distanceSquared = squaredDistance(towerX, towerY, enemyX, enemyY);
        if (distanceSquared > rangeSquared || distanceSquared >= nearestDistanceSquared) continue;

        nearestDistanceSquared = distanceSquared;
        nearestEnemyIndex = i;
    }

    return nearestEnemyIndex;
}

std::optional<std::size_t> TowerManager::findTowerIndex(int gridX, int gridY) const {
    for (std::size_t i = 0; i < towers.size(); ++i) {
        if (towers[i].GetGridX() == gridX && towers[i].GetGridY() == gridY) return i;
    }
    return std::nullopt;
}

void TowerManager::setAutoAttackEnabled(bool enabled) { autoAttackEnabled = enabled; }
bool TowerManager::getAutoAttackEnabled() const { return autoAttackEnabled; }

void TowerManager::updateAutoAttack(float deltaTime, std::vector<Enemy>& enemies) {
    if (deltaTime <= 0.0F) return;

    for (float& cooldown : towerCooldowns) {
        cooldown = std::max(0.0F, cooldown - deltaTime);
    }

    if (autoAttackEnabled) {
        static constexpr float kWeaponAngleOffset = -1.5707963F;

        for (std::size_t i = 0; i < towers.size(); ++i) {
            if (towerCooldowns[i] > 0.0F) continue;

            const Tower& tower = towers[i];
            const TowerDef& def = getTowerDef(towerIds[i]);
            const std::optional<std::size_t> enemyIndexOpt = findNearestEnemyIndex(tower, enemies);
            if (!enemyIndexOpt.has_value()) continue;

            const Enemy& targetEnemy = enemies[enemyIndexOpt.value()];
            const float towerX = static_cast<float>(tower.GetGridX());
            const float towerY = static_cast<float>(tower.GetGridY());
            const float dx = targetEnemy.getX() - towerX;
            const float dy = targetEnemy.getY() - towerY;
            const float distance = std::sqrt(dx * dx + dy * dy);
            if (distance <= 0.0001F) continue;

            towers[i].SetFacingRotation(std::atan2(dy, dx) + kWeaponAngleOffset);

            Projectile projectile;
            projectile.x = towerX;
            projectile.y = towerY;
            projectile.dirX = dx / distance;
            projectile.dirY = dy / distance;
            projectile.speed = def.projectileSpeed;
            projectile.damage = def.damage;
            projectile.lifetime = 2.0F;
            projectile.splashRadius = def.splashRadius;
            projectiles.push_back(projectile);
            towerCooldowns[i] = def.fireInterval;
        }
    }

    // 修正 Clang-Tidy：使用 auto
    for (auto i = static_cast<std::ptrdiff_t>(projectiles.size()) - 1; i >= 0; --i) {
        Projectile& projectile = projectiles[static_cast<std::size_t>(i)];
        projectile.lifetime -= deltaTime;
        if (projectile.lifetime <= 0.0F) {
            projectiles.erase(projectiles.begin() + i);
            continue;
        }

        // ── 尋向追蹤：朝最近敵人轉向 ──
        std::optional<std::size_t> nearestEnemyIndex = std::nullopt;
        float nearestEnemyDistanceSquared = std::numeric_limits<float>::max();
        for (std::size_t enemyIndex = 0; enemyIndex < enemies.size(); ++enemyIndex) {
            const Enemy& enemy = enemies[enemyIndex];
            if (!enemy.isAlive() || enemy.hasReachedGoal()) continue;

            const float distanceSquared = squaredDistance(projectile.x, projectile.y, enemy.getX(), enemy.getY());
            if (distanceSquared >= nearestEnemyDistanceSquared ||
                distanceSquared > (kProjectileSeekRadius * kProjectileSeekRadius)) {
                continue;
            }
            nearestEnemyDistanceSquared = distanceSquared;
            nearestEnemyIndex = enemyIndex;
        }

        if (nearestEnemyIndex.has_value()) {
            Enemy& target = enemies[nearestEnemyIndex.value()];
            const float dx = target.getX() - projectile.x;
            const float dy = target.getY() - projectile.y;
            const float distance = std::sqrt(dx * dx + dy * dy);

            if (distance <= kProjectileHitRadius) {
                // 命中目標（在移動前偵測到）
                if (projectile.splashRadius > 0.0F) {
                    const float splashSq = projectile.splashRadius * projectile.splashRadius;
                    for (Enemy& splashEnemy : enemies) {
                        if (!splashEnemy.isAlive() || splashEnemy.hasReachedGoal()) continue;
                        if (squaredDistance(projectile.x, projectile.y, splashEnemy.getX(), splashEnemy.getY()) <= splashSq) {
                            splashEnemy.takeDamage(projectile.damage);
                        }
                    }
                } else {
                    target.takeDamage(projectile.damage);
                }
                projectiles.erase(projectiles.begin() + i);
                continue;
            }

            if (distance > 0.0001F) {
                projectile.dirX = dx / distance;
                projectile.dirY = dy / distance;
            }
        }

        // ── 移動 ──
        const float moveDistance = projectile.speed * deltaTime;
        projectile.x += projectile.dirX * moveDistance;
        projectile.y += projectile.dirY * moveDistance;

        // ── 命中偵測（移動後） ──
        bool hasHitEnemy = false;
        for (Enemy& enemy : enemies) {
            if (!enemy.isAlive() || enemy.hasReachedGoal()) continue;
            if (squaredDistance(projectile.x, projectile.y, enemy.getX(), enemy.getY()) <=
                (kProjectileHitRadius * kProjectileHitRadius)) {
                if (projectile.splashRadius > 0.0F) {
                    // ★ 爆炸：對範圍內所有敵人造成傷害
                    const float splashSq = projectile.splashRadius * projectile.splashRadius;
                    for (Enemy& splashEnemy : enemies) {
                        if (!splashEnemy.isAlive() || splashEnemy.hasReachedGoal()) continue;
                        if (squaredDistance(projectile.x, projectile.y, splashEnemy.getX(), splashEnemy.getY()) <= splashSq) {
                            splashEnemy.takeDamage(projectile.damage);
                        }
                    }
                } else {
                    // 單體傷害
                    enemy.takeDamage(projectile.damage);
                }
                hasHitEnemy = true;
                break;
            }
        }

        if (hasHitEnemy) {
            projectiles.erase(projectiles.begin() + i);
        }
    }
}
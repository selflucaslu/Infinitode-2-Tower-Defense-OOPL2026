#include "enemy/Enemy.hpp"

#include <cmath>
#include <utility>

Enemy::Enemy(float speed, int maxHealth, int damage, std::string spriteId, float startX, float startY, int startPathIndex) : x(startX), y(startY), pathIndex(startPathIndex), speed(speed), health(maxHealth), maxHealth(maxHealth), damage(damage), spriteId(std::move(spriteId)) {}

float Enemy::getX() const {
    return x;
}

float Enemy::getY() const {
    return y;
}

int Enemy::getPathIndex() const {
    return pathIndex;
}

float Enemy::getSpeed() const {
    return speed;
}

int Enemy::getHealth() const {
    return health;
}

int Enemy::getMaxHealth() const {
    return maxHealth;
}

int Enemy::getDamage() const {
    return damage;
}

std::string Enemy::getSpriteId() const {
    return spriteId;
}

void Enemy::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

void Enemy::setPathIndex(int newPathIndex) {
    pathIndex = newPathIndex;
}

// -------------------- 更新敵人移動 --------------------
void Enemy::update(float deltaTime, const std::vector<std::pair<float, float>>& pathPoints) {
    // deltaTime = 這一幀經過的秒數（例如 60 FPS 時約 0.016）。
    // 沒路徑、已死亡、已到終點 -> 不更新
    if (pathPoints.empty() || !isAlive() || reachedGoal) {
        return;
    }

    // 安全檢查：索引超出代表已走完
    if (pathIndex < 0 || pathIndex >= static_cast<int>(pathPoints.size())) {
        reachedGoal = true;
        return;
    }

    // 目前要前進到的目標點
    const float targetX = pathPoints[pathIndex].first;
    const float targetY = pathPoints[pathIndex].second;
    const float dx = targetX - x;
    const float dy = targetY - y;
    const float distance = std::sqrt(dx * dx + dy * dy);

    // speed 是「每秒速度」，所以要乘上 deltaTime 才是「這一幀可移動距離」。
    // 例：speed=120（每秒 120 單位），deltaTime=0.016（約 60 FPS）-> 本幀移動約 1.92 單位。
    const float moveDistance = speed * deltaTime;

    // 本幀就能到目標點：直接貼齊並換下一點
    if (distance <= 0.0001F || moveDistance >= distance) {
        x = targetX;
        y = targetY;
        pathIndex++;
        if (pathIndex >= static_cast<int>(pathPoints.size())) {
            reachedGoal = true;
        }
        return;
    }

    // 還到不了目標點：往目標方向前進
    x += (dx / distance) * moveDistance;
    y += (dy / distance) * moveDistance;
}

void Enemy::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
}

bool Enemy::isAlive() const {
    return health > 0;
}

bool Enemy::hasReachedGoal() const {
    return reachedGoal;
}

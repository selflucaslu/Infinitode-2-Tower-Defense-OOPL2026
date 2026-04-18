#include "enemy/Enemy.hpp"

#include <cmath>
#include <utility>

// -------------------- 建立單一敵人 --------------------
Enemy::Enemy(
    float startX,
    float startY,
    float speed,
    MoveType moveType,
    int maxHealth,
    int damage,
    int rewardGold,
    int startPathIndex,
    std::string spriteId,
    std::shared_ptr<const std::vector<std::pair<int, int>>> pathPoints
)
    : rewardGold(rewardGold),
      x(startX),
      y(startY),
      speed(speed),
      moveType(moveType),
      maxHealth(maxHealth),
      health(maxHealth),
      damage(damage),
      pathIndex(startPathIndex),
      pathPoints(std::move(pathPoints)),
      spriteId(std::move(spriteId)) {
}

// -------------------- 基本查詢 --------------------
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

Enemy::MoveType Enemy::getMoveType() const {
    return moveType;
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

int Enemy::getRewardGold() const {
    return rewardGold;
}

std::string_view Enemy::getSpriteId() const {
    return spriteId;
}

// -------------------- 基本設定 --------------------
void Enemy::setPosition(float newX, float newY) {
    x = newX;
    y = newY;
}

void Enemy::setPathIndex(int newPathIndex) {
    pathIndex = newPathIndex;
}

// -------------------- 更新敵人移動 --------------------
void Enemy::update(float deltaTime) {
    // deltaTime = 這一幀經過的秒數（例如 60 FPS 時約 0.016）。
    // 沒路徑、已死亡、已到終點 -> 不更新
    if (!pathPoints || pathPoints->empty() || !isAlive() || reachedGoal) {
        return;
    }

    // shared_ptr 綁定的固定路徑，於建構時注入。
    const std::vector<std::pair<int, int>>& path = *pathPoints;

    // 安全檢查：索引超出代表已走完
    if (pathIndex < 0 || pathIndex >= static_cast<int>(path.size())) {
        reachedGoal = true;
        return;
    }

    // 目前要前進到的目標點
    const float targetX = path[pathIndex].first;
    const float targetY = path[pathIndex].second;
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
        if (pathIndex >= static_cast<int>(path.size())) {
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

// -------------------- 狀態判斷 --------------------
bool Enemy::isAlive() const {
    return health > 0;
}

bool Enemy::hasReachedGoal() const {
    return reachedGoal;
}

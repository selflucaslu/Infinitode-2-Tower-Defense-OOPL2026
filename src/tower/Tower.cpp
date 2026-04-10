#include "tower/Tower.hpp"

#include <utility>

// -------------------- 建立塔 --------------------
Tower::Tower(int gridX, int gridY, std::string spriteId)
    : gridX(gridX),
      gridY(gridY),
      spriteId(std::move(spriteId)),
      range(2.5F),
      attackDamage(10),
      attackInterval(0.5F),
      attackCooldown(0.0F) {
}

// -------------------- 讀取屬性 --------------------
int Tower::getGridX() const {
    return gridX;
}

int Tower::getGridY() const {
    return gridY;
}

std::string Tower::getSpriteId() const {
    return spriteId;
}

float Tower::getRange() const {
    return range;
}

int Tower::getAttackDamage() const {
    return attackDamage;
}

float Tower::getAttackInterval() const {
    return attackInterval;
}

float Tower::getAttackCooldown() const {
    return attackCooldown;
}

// -------------------- 寫入屬性 --------------------
void Tower::setGridPosition(int newGridX, int newGridY) {
    gridX = newGridX;
    gridY = newGridY;
}

void Tower::setAttackCooldown(float newCooldown) {
    attackCooldown = newCooldown;
}

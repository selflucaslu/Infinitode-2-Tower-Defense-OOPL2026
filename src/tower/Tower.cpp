#include "tower/Tower.hpp"
#include <utility>
#include <stdexcept>

Tower::Tower(std::string spriteId, int gridX, int gridY, float attack_range, int Level, TowerType type):
spriteId(spriteId), Level(Level), gridX(gridX), gridY(gridY), attack_range(attack_range), type(type) {
}

Tower::~Tower()
{
}

std::string Tower::GetspriteId() const
{
    return spriteId;
}

int Tower::GetGridX() const
{
    return gridX;
}

int Tower::GetGridY() const
{
    return gridY;
}

float Tower::GetAttackRange() const
{
    return attack_range;
}

int Tower::GetLevel() const
{
    return Level;
}

Tower::TowerType Tower::GetTypes() const
{
    return type;
}

void Tower::SetAttackRange(float new_attack_range)
{
    attack_range=(new_attack_range>0)? new_attack_range : throw std::invalid_argument("illegal value");
}

void Tower::SetLevel(int new_Level)
{
    Level=new_Level>0?new_Level:throw std::invalid_argument("illegal value");
}
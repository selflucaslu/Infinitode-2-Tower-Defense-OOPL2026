#include "tower/Tower.hpp"
#include <utility>
Tower::Tower(std::string TowerId, int gridX, int gridY, float attack_range, int Level, TowerType type):
TowerId(TowerId),attack_range(attack_range),Level(Level),type(type),
{
}

Tower::~Tower()
{
}

std::string Tower::GetTowerID() const
{
    return TowerId;
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
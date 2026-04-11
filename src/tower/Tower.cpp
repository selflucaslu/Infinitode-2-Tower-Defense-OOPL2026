#include "tower/Tower.hpp"
#include <utility>
Tower::Tower(std::string TowerId, int originX, int originY, float attack_range, int Level, TowerType type):
TowerId(TowerId),originX(originX),originY(originY),attack_range(attack_range),Level(Level),type(type)
{
}

Tower::~Tower()
{
}

int Tower::GetX() const
{
    return originX;
}

int Tower::GetY() const
{
    return originY;
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

void Tower::setPosition(float newX, float newY)
{
    originX=(newX>=0)? newX :throw std::invalid_argument("illegal value");
    originY=(newY>=0)? newY :throw std::invalid_argument("illegal value");
}
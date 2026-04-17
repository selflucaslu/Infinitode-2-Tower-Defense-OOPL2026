#include <tower/AmmoTower.hpp>

AmmoTower::AmmoTower(std::string spriteId, float AmmoSpeed, float AmmoPower):spriteId(spriteId),AmmoSpeed(AmmoSpeed),AmmoPower(AmmoPower)
{

}

AmmoTower::~AmmoTower()
{

}

std::string AmmoTower::getSpriteId()
{
    return spriteId;
}

float AmmoTower::getAmmoPower()
{
    return AmmoPower;
}

float AmmoTower::getAmmoSpeed()
{
    return AmmoSpeed;
}

void AmmoTower::setAmmoPower(float newPower)
{
    this->AmmoPower=newPower;
}

void AmmoTower::setAmmoSpeed(float newSpeed)
{
    this->AmmoSpeed=newSpeed;
}

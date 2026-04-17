#pragma once
#include <tower/Tower.hpp>

class AmmoTower
{
    std::string spriteId;
    float AmmoSpeed=10.0f,AmmoPower=10.0f;//子彈速度、子彈物理傷害(子彈力道)
public:
    AmmoTower(std::string spriteId,float AmmoSpeed,float AmmoPower);
    virtual ~AmmoTower();
    virtual std::string getSpriteId();
    virtual float getAmmoSpeed();
    virtual float getAmmoPower();

    virtual void setAmmoSpeed(float newSpeed);
    virtual void setAmmoPower(float newPower);
};
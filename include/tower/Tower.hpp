#pragma once

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <memory>

class Tower{
    enum TowerType {
        AMMO,//只要會射都叫AMMO
        AROUND_SKILL//用範圍技的都叫AROUND_SKILL
    }type;
    std::string TowerId;
    int originX=0,originY=0,Level=1;
    float attack_range=10.0f;
    public:
        Tower(std::string Tower,int originX=0,int originY=0,float attack_range=10.0f,int Level=1,TowerType type=AMMO);
        ~Tower();
        std::string GetTowerID() const;
        virtual int GetX() const;
        virtual int GetY() const;
        virtual float GetAttackRange() const;
        virtual int GetLevel() const;
        virtual TowerType GetTypes() const;

        virtual void setPosition(float newX, float newY);
        virtual void SetAttackRange(float new_attack_range);
        virtual void SetLevel(int new_Level);
};
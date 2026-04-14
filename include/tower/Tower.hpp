#pragma once

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include <memory>
#include <string>

class Tower{
    std::string TowerId = "tower-basic";
    int Level=1;
    int gridX=0; // 格子 x
    int gridY=0; // 格子 y
    float attack_range=10.0f;
    public:
        enum TowerType {
        AMMO,//只要會射都叫AMMO
        AROUND_SKILL//用範圍技的都叫AROUND_SKILL
        }type;
        Tower(std::string TowerId,int gridX=0,int gridY=0,float attack_range=10.0f,int Level=1,TowerType type=AMMO);
        ~Tower();
        std::string GetTowerID() const;
        virtual int GetGridX() const;
        virtual int GetGridY() const;
        virtual float GetAttackRange() const;
        virtual int GetLevel() const;
        virtual TowerType GetTypes() const;

        virtual void SetAttackRange(float new_attack_range);
        virtual void SetLevel(int new_Level);
};
#pragma once

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "map/GridMap.hpp"
#include <memory>
#include <string>

class Tower{
    std::string spriteId = "tower-basic";
    int Level; //砲台等級
    int gridX; // 格子 x
    int gridY; // 格子 y
    float facingRotation; // 砲口目前朝向（弧度）
    float attack_range;//攻擊範圍
    float Traverse_Speed;//砲台轉向速度
    public:
        enum TowerType {
        AMMO,//只要會射都叫AMMO
        AROUND_SKILL//用範圍技的都叫AROUND_SKILL
        }type;
        Tower(std::string spriteId,int gridX=0,int gridY=0,float attack_range=10.0f,int Level=1,TowerType type=AMMO);
        virtual ~Tower();
        std::string GetspriteId() const;
        virtual int GetGridX() const;
        virtual int GetGridY() const;
        virtual float GetFacingRotation() const;
        virtual float GetAttackRange() const;
        virtual int GetLevel() const;
        virtual TowerType GetTypes() const;

        virtual void SetFacingRotation(float newFacingRotation);
        virtual void SetAttackRange(float new_attack_range);
        virtual void SetLevel(int new_Level);
};

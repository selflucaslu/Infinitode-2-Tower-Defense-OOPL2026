#pragma once

#include <string>

class Tower {
public:
    // 塔的格子位置與貼圖 ID。
    Tower(
        int gridX = 0,
        int gridY = 0,
        std::string spriteId = "tower-basic"
    );

    int getGridX() const; // 塔所在格子 x
    int getGridY() const; // 塔所在格子 y
    std::string getSpriteId() const; // 塔貼圖 ID
    float getRange() const; // 攻擊範圍
    int getAttackDamage() const; // 直傷傷害
    float getAttackInterval() const; // 攻擊間隔（秒）
    float getAttackCooldown() const; // 目前冷卻（秒）

    void setGridPosition(int newGridX, int newGridY); // 變更塔位置
    void setAttackCooldown(float newCooldown); // 設定冷卻值

private:
    int gridX; // 格子 x
    int gridY; // 格子 y
    std::string spriteId; // 貼圖 ID
    float range; // 攻擊範圍（由 cpp 初始化）
    int attackDamage; // 直傷傷害（由 cpp 初始化）
    float attackInterval; // 攻擊間隔（由 cpp 初始化）
    float attackCooldown; // 冷卻計時（由 cpp 初始化）
};

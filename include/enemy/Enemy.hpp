#pragma once

#include <string>
#include <utility>
#include <vector>

class Enemy {
public:
    Enemy(
        float speed = 1.0F,
        int maxHealth = 40,
        int damage = 5,
        std::string spriteId = "enemy-type-regular",
        float startX = 0.0F,
        float startY = 0.0F,
        int startPathIndex = 0
    );

    float getX() const;
    float getY() const;
    int getPathIndex() const;
    float getSpeed() const;
    int getHealth() const;
    int getMaxHealth() const;
    int getDamage() const;
    std::string getSpriteId() const;

    void setPosition(float newX, float newY);
    void setPathIndex(int newPathIndex);

    void update(float deltaTime, const std::vector<std::pair<float, float>>& pathPoints);
    void takeDamage(int amount);
    bool isAlive() const;
    bool hasReachedGoal() const;

private:
    float x = 0.0F;        // 世界座標 x（平滑移動用 float）
    float y = 0.0F;        // 世界座標 y（平滑移動用 float）
    int pathIndex = 0;     // 目前目標路徑節點索引
    float speed = 1.0F;    // 每秒移動速度
    int health = 40;       // 目前血量
    int maxHealth = 40;    // 血量上限
    int damage = 5;        // 到終點時造成的傷害
    std::string spriteId = "enemy-type-regular"; // 圖片資源 ID
    bool reachedGoal = false; // 是否已經走到終點
};

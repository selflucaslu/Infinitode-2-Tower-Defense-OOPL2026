#pragma once

#include <string>
#include <utility>
#include <vector>

class Enemy {
public:
    // 單一敵人基礎資料。
    Enemy(
        float startX = 0.0F,
        float startY = 0.0F,
        float speed = 1.0F,
        int maxHealth = 40,
        int damage = 5,
        int startPathIndex = 0,
        std::string spriteId = "enemy-type-regular"
    );

    float getX() const; // 取得世界座標 x
    float getY() const; // 取得世界座標 y
    int getPathIndex() const; // 取得目前路徑節點索引
    float getSpeed() const; // 取得移動速度
    int getHealth() const; // 取得目前血量
    int getMaxHealth() const; // 取得最大血量
    int getDamage() const; // 取得到終點造成傷害
    std::string getSpriteId() const; // 取得貼圖 ID

    void setPosition(float newX, float newY); // 設定世界座標
    void setPathIndex(int newPathIndex); // 設定路徑索引

    void update(float deltaTime, const std::vector<std::pair<float, float>>& pathPoints); // 沿路徑更新移動
    void takeDamage(int amount); // 扣血
    bool isAlive() const; // 是否存活
    bool hasReachedGoal() const; // 是否已抵達終點

private:
    float x;        // 世界座標 x（平滑移動用 float）
    float y;        // 世界座標 y（平滑移動用 float）
    float speed;    // 每秒移動速度
    int maxHealth;    // 血量上限
    int health;       // 目前血量
    int damage;        // 到終點時造成的傷害
    int pathIndex;     // 目前目標路徑節點索引
    bool reachedGoal = false; // 是否已經走到終點
    std::string spriteId = "enemy-type-regular"; // 圖片資源 ID
};

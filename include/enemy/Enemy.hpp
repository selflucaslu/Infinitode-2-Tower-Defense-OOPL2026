#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

class Enemy {
public:
    enum class MoveType {
        Ground,
        Air
    };

    // -------------------- 建構 --------------------
    // 單一敵人基礎資料。
    // pathPoints 可在建構時綁定固定路徑（shared_ptr 共享，避免每隻敵人複製整條路徑）。
    Enemy(
        float startX,
        float startY,
        float speed,
        MoveType moveType,
        int maxHealth,
        int damage,
        int startPathIndex,
        std::string spriteId,
        std::shared_ptr<const std::vector<std::pair<int, int>>> pathPoints
    );

    // -------------------- 查詢 --------------------
    float getX() const; // 取得世界座標 x
    float getY() const; // 取得世界座標 y
    int getPathIndex() const; // 取得目前路徑節點索引
    float getSpeed() const; // 取得移動速度
    MoveType getMoveType() const; // 取得移動類型
    int getHealth() const; // 取得目前血量
    int getMaxHealth() const; // 取得最大血量
    int getDamage() const; // 取得到終點造成傷害
    std::string getSpriteId() const; // 取得貼圖 ID

    // -------------------- 設定 --------------------
    void setPosition(float newX, float newY); // 設定世界座標
    void setPathIndex(int newPathIndex); // 設定路徑索引

    // -------------------- 行為與狀態 --------------------
    void update(float deltaTime); // 沿路徑更新移動
    void takeDamage(int amount); // 扣血
    bool isAlive() const; // 是否存活
    bool hasReachedGoal() const; // 是否已抵達終點

private:
    // -------------------- 位置與移動 --------------------
    float x;        // 世界座標 x（平滑移動用 float）
    float y;        // 世界座標 y（平滑移動用 float）
    float speed;    // 每秒移動速度
    MoveType moveType; // 移動類型（地面或飛行）

    // -------------------- 戰鬥屬性 --------------------
    int maxHealth;    // 血量上限
    int health;       // 目前血量
    int damage;        // 到終點時造成的傷害

    // -------------------- 路徑狀態 --------------------
    int pathIndex;     // 目前目標路徑節點索引
    std::shared_ptr<const std::vector<std::pair<int, int>>> pathPoints; // 該敵人的固定路徑
    bool reachedGoal = false; // 是否已經走到終點

    // -------------------- 顯示資源 --------------------
    std::string spriteId; // 圖片資源 ID
};

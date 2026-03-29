#pragma once

#include <string>


class Tile {
public:
    Tile(std::string spriteId);
    std::string getSpriteId() const;
    bool getIsWalkable() const;
    bool getIsBuildable() const;
private:
    std::string spriteId;
    bool isWalkable;   // 可以行走地形
    bool isBuildable;  // 可以建造地形
    // int flags = 0;  // Future use for additional properties
};

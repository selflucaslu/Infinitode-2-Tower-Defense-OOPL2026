#include "map/Tile.hpp"

#include <utility>

Tile::Tile(std::string spriteId) : spriteId(std::move(spriteId)) {
    // 建塔平台（不可行走）
    if (spriteId == "tile-type-platform") {
        isWalkable = false;
        isBuildable = true;
    } else {
        isWalkable = true;
        isBuildable = false;
    }
}

std::string Tile::getSpriteId() const {
    return spriteId;
}

bool Tile::getIsWalkable() const {
    return isWalkable;
}

bool Tile::getIsBuildable() const {
    return isBuildable;
}

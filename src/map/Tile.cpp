#include "map/Tile.hpp"

#include <string_view>
#include <utility>

namespace {
bool startsWith(const std::string& text, std::string_view prefix) {
    return text.rfind(prefix, 0) == 0;
}
} // namespace

Tile::Tile(std::string spriteId) : spriteId(std::move(spriteId)) {
    isSpawn = startsWith(this->spriteId, "tile-type-spawn-");
    isTarget = startsWith(this->spriteId, "tile-type-target-");
    isRoad = startsWith(this->spriteId, "tile-type-road-");
    isWall = this->spriteId == "tile-type-platform";

    isBuildable = isWall;
    isWalkable = !isWall;
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

bool Tile::isSpawnTile() const {
    return isSpawn;
}

bool Tile::isTargetTile() const {
    return isTarget;
}

bool Tile::isRoadTile() const {
    return isRoad;
}

bool Tile::isWallTile() const {
    return isWall;
}

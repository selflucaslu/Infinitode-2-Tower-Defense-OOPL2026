#include "map/Tile.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

Tile::Tile(std::string spriteId)
    : spriteId(std::move(spriteId)), type(parseTypeFromSpriteId(this->spriteId)) {
}

std::string Tile::getSpriteId() const {
    return spriteId;
}

Tile::Type Tile::getType() const {
    return type;
}

namespace {
bool hasPrefix(std::string_view text, std::string_view prefix) {
    return text.rfind(prefix, 0) == 0;
}
} // namespace

Tile::Type Tile::parseTypeFromSpriteId(std::string_view spriteId) const {
    // 識別邏輯採「前綴比對」：
    // 1) 先比對可建塔地：tile-type-platform* -> Platform
    // 2) 再比對道路：tile-type-road* -> Road
    // 3) 起點相關（spawn / source）-> Spawn
    // 4) 終點相關（target / game-value-base）-> Goal
    // 5) 明確牆體：tile-type-wall* -> Wall
    //
    // 涵蓋範圍：
    // - combined.atlas 中目前地圖會用到的 road/platform/spawn/target 系列皆可被識別。
    // - 其餘未列入上述規則的 tile-type-* 一律視為 Wall，避免被誤判為可走或可建塔地。
    // - tile-type-empty 為空白格（不渲染、不可走、不可建）
    if (hasPrefix(spriteId, "tile-type-empty")) {
        return Type::Empty;
    }

    if (hasPrefix(spriteId, "tile-type-platform")) {
        return Type::Platform;
    }
    if (hasPrefix(spriteId, "tile-type-road")) {
        return Type::Road;
    }
    if (hasPrefix(spriteId, "tile-type-spawn") || hasPrefix(spriteId, "tile-type-source")) {
        return Type::Spawn;
    }
    if (hasPrefix(spriteId, "tile-type-target") || spriteId == "tile-type-game-value-base") {
        return Type::Goal;
    }
    if (hasPrefix(spriteId, "tile-type-wall")) {
        return Type::Wall;
    }

// Debug 模式下，若遇到未知的 tile 類型，會丟出例外，正式版本則一律視為 Wall
#ifndef NDEBUG
    if (hasPrefix(spriteId, "tile-type-")) {
        throw std::runtime_error("未知 tile 類型: " + std::string(spriteId));
    }
    throw std::runtime_error("非法 tile spriteId: " + std::string(spriteId));
#else
    return Type::Wall;
#endif
}

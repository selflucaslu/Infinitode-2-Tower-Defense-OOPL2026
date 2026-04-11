#pragma once

#include <string>
#include <string_view>

class Tile {
public:
    // Tile 類型：用於外部判斷道路、建塔地、起終點與牆。
    enum class Type {
        Road,
        Platform,
        Wall,
        Spawn,
        Goal,
    };

    explicit Tile(std::string spriteId);
    std::string getSpriteId() const; // 回傳對應貼圖 ID
    Type getType() const; // 回傳該格邏輯類型（外部統一以 Type 判斷）

private:
    // 由貼圖 ID 解析成 Tile 類型。
    Type parseTypeFromSpriteId(std::string_view spriteId) const;

    std::string spriteId; // 該格貼圖 ID
    Type type; // 該格類型（由 spriteId 解析）
};

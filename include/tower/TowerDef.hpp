#pragma once

#include <string>
#include <vector>
#include <array>
#include <string_view>

// -------------------- 塔種類 ID --------------------
enum class TowerId {
    Basic,       // 普通射擊塔（低費）
    Sniper,      // 狙擊塔（高傷害、慢射速、遠射程）
    Cannon,      // 加農炮塔（範圍傷害、中高傷害）
};

// -------------------- 塔種類設定 --------------------
struct TowerDef {
    TowerId id;
    std::string_view displayName;   // 顯示名稱（HUD 用）
    std::string_view spriteBase;    // 底座 sprite ID
    std::string_view spriteWeapon;  // 砲管 sprite ID
    std::string_view spriteProjectile; // 子彈 sprite ID
    int buildCost;                  // 建造費用
    float attackRange;              // 攻擊範圍（格子數）
    float fireInterval;             // 射擊間隔（秒）
    int damage;                     // 每次傷害
    float projectileSpeed;          // 子彈速度（格/秒）
    float splashRadius;             // 爆炸半徑（0 = 無爆炸）
};

// -------------------- 所有塔種類的靜態資料表 --------------------
inline const std::array<TowerDef, 3> kTowerDefs = {{
    {
        TowerId::Basic,
        "Basic",
        "tower-basic-base",
        "tower-basic-weapon",
        "projectile-basic",
        40,     // 建造費用
        7.0F,   // 攻擊範圍
        0.35F,  // 射擊間隔
        12,     // 傷害
        9.0F,   // 子彈速度
        0.0F,   // 無爆炸
    },
    {
        TowerId::Sniper,
        "Sniper",
        "tower-sniper-base-new",
        "tower-sniper-weapon-new",
        "projectile-basic",
        80,     // 建造費用
        14.0F,  // 攻擊範圍（更遠）
        1.20F,  // 射擊間隔（較慢）
        60,     // 傷害（更高）
        16.0F,  // 子彈速度（更快）
        0.0F,   // 無爆炸
    },
    {
        TowerId::Cannon,
        "Cannon",
        "tower-cannon-base-new",
        "tower-cannon-weapon-new",
        "projectile-basic",
        100,    // 建造費用
        6.0F,   // 攻擊範圍（稍近）
        1.50F,  // 射擊間隔（慢）
        40,     // 傷害（中高）
        7.0F,   // 子彈速度（稍慢）
        1.5F,   // 爆炸半徑（格子數）
    },
}};

// -------------------- 工具函式 --------------------
inline const TowerDef& getTowerDef(TowerId id) {
    for (const TowerDef& def : kTowerDefs) {
        if (def.id == id) return def;
    }
    return kTowerDefs[0]; // fallback
}

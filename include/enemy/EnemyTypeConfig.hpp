#pragma once

#include "enemy/Enemy.hpp"

#include <array>
#include <string_view>

// -------------------- 敵人類型識別 --------------------
// 目前先支援敵人：
// Regular / Fast / Strong / Heli / Jet
// 若未來要擴充 Armored / Healer / Toxic...，請在 Jet 後方追加。
enum class EnemyTypeId {
    Regular,
    Fast,
    Strong,
    Heli,
    Jet,
    // 預留擴充類型（目前未啟用）
    Armored,
    Healer,
    Toxic,
    Icy,
    Fighter,
    Light,
    Broot,
    Count
};

// -------------------- 敵人配置資料 --------------------
struct EnemyTypeConfig {
    // 識別與顯示
    EnemyTypeId id = EnemyTypeId::Regular; // 敵人類型 ID（程式內主要識別鍵）
    std::string_view name = "Regular"; // 顯示名稱（UI / 除錯用）
    std::string_view spriteId = "enemy-type-regular"; // Atlas 裡對應的貼圖 key
    Enemy::MoveType moveType = Enemy::MoveType::Ground; // 移動型態（Ground/Air）

    // 基礎數值
    int maxHealth = 40;   // 血量上限
    float speed = 1.0F;   // 移動速度（以格子/秒為近似語意）
    int damageToBase = 1; // 抵達終點時對基地造成的傷害
    int rewardGold = 2;   // 被擊殺時給玩家的金幣

    // 通用減傷 / 護甲
    int flatArmor = 0; // 每次傷害固定扣減（例如 10 傷害先減 2）
    // -------------------- 預留擴充欄位（暫未啟用） --------------------
    float damageTakenMultiplier = 1.0F; // 乘算減傷（1.0=正常、0.7=僅承受 70%）

    // Aura 類（影響周圍敵人）
    bool hasShieldAura = false;       // 是否啟用減傷 Aura（Armored 類常用）
    float shieldAuraRadius = 0.0F;    // 減傷 Aura 半徑（格子座標單位）
    float shieldAuraReduction = 0.0F; // 減傷比例（0.35 = 周圍減傷 35%）

    bool hasHealAura = false;          // 是否啟用治療 Aura（Healer 類常用）
    float healAuraRadius = 0.0F;       // 治療 Aura 半徑（格子座標單位）
    float healPercentPerSecond = 0.0F; // 每秒回復比例（0.04 = 每秒回 4%）

    // 自回復（Toxic 類常用）
    bool selfRegenWhenUntouched = false; // 是否啟用「未受傷一段時間後自回」
    float regenDelaySeconds = 0.0F;      // 幾秒未受傷後開始回復
    float regenPercentPerSecond = 0.0F;  // 每秒回復比例

    // 免疫 / 特性
    bool immunePoison = false; // 是否免疫中毒
    bool immuneSlow = false;   // 是否免疫緩速
    bool immuneStun = false;   // 是否免疫暈眩

    // 可破壞護盾（Icy 類常用）
    bool hasBreakableShield = false; // 是否有獨立護盾條
    int shieldHealth = 0;            // 護盾血量（先扣盾再扣本體）

    // 分裂（Fighter 類常用）
    bool splitsOnDeath = false; // 死亡是否觸發分裂
    int splitCount = 0;         // 分裂出的子敵人數量
    EnemyTypeId splitChildType = EnemyTypeId::Regular; // 子敵人型態
    float splitChildHealthFactor = 0.0F; // 子敵人血量倍率（0.45 = 45%）

    // 適應抗性（Light 類常用）
    bool adaptiveResistance = false;         // 是否啟用「最近傷害類型抗性」
    float adaptiveResistancePercent = 0.0F;  // 抗性比例（0.30 = 減傷 30%）
    float adaptiveResistanceDuration = 0.0F; // 抗性持續時間（秒）

    // Boss
    bool isBoss = false;                  // 是否為 Boss 單位
    bool rageOnLowHp = false;             // 低血是否進入狂暴
    float rageHpThreshold = 0.0F;         // 狂暴觸發血量比例（0.25 = 25%）
    float rageDuration = 0.0F;            // 狂暴持續時間（秒）
    float rageSpeedMultiplier = 1.0F;     // 狂暴速度倍率（<1.0 更慢、>1.0 更快）
    float rageHealFromDamageRatio = 0.0F; // 狂暴期間受傷轉回血比例（0.50 = 50%）
};

inline int toIndex(EnemyTypeId id) {
    return static_cast<int>(id);
}

// -------------------- 全域敵人配置表 --------------------
inline const std::array<EnemyTypeConfig, static_cast<int>(EnemyTypeId::Count)> kEnemyTypeConfigs = {{
    // -------------------- Regular --------------------
    // 角色風格：最基礎、無特殊效果的標準敵人，用來建立玩家節奏。
    // 參考摘要：第一個敵人，約 1 tile/s，抵達基地造成 1 傷害。
    EnemyTypeConfig{
        EnemyTypeId::Regular,
        "Regular",
        "enemy-type-regular",
        Enemy::MoveType::Ground,
        40,
        1.00F,
        1,
        2,
    },

    // -------------------- Fast --------------------
    // 角色風格：速度壓力型，血量較低但可快速穿過慢速高傷防線。
    // 參考摘要：約比 Regular 快 25%，屬「快但脆」敵人。
    EnemyTypeConfig{
        EnemyTypeId::Fast,
        "Fast",
        "enemy-type-fast",
        Enemy::MoveType::Ground,
        25,
        1.25F,
        1,
        2,
    },

    // -------------------- Strong --------------------
    // 角色風格：前期坦型壓力，血量高、速度較慢，通常需要高單體輸出應對。
    // 參考摘要：高血量常見威脅，對部分塔有明顯抗性。
    EnemyTypeConfig{
        EnemyTypeId::Strong,
        "Strong",
        "enemy-type-strong",
        Enemy::MoveType::Ground,
        85,
        0.82F,
        2,
        3,
        1,
    },

    // -------------------- Heli --------------------
    // 角色風格：第一個飛行壓力，主要用來檢查玩家是否有對空手段。
    // 參考摘要：「由空中移動，多數塔無法攻擊」。
    EnemyTypeConfig{
        EnemyTypeId::Heli,
        "Heli",
        "enemy-type-heli",
        Enemy::MoveType::Air,
        55,
        1.00F,
        1,
        3,
    },

    // -------------------- Jet --------------------
    // 角色風格：高速飛行壓力，通常視作 Fast + Heli 的混合型節奏考驗。
    // 參考摘要：飛行且高速，血量低於 Heli 但更容易造成漏怪。
    EnemyTypeConfig{
        EnemyTypeId::Jet,
        "Jet",
        "enemy-type-jet",
        Enemy::MoveType::Air,
        35,
        1.45F,
        1,
        3,
    }
}};

inline const EnemyTypeConfig& getEnemyTypeConfig(EnemyTypeId id) {
    return kEnemyTypeConfigs[toIndex(id)];
}

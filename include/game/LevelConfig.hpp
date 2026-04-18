#pragma once

#include "enemy/EnemyTypeConfig.hpp"

#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

// -------------------- 單組生怪配置 --------------------
// 說明：一個 Wave 內可有多個 SpawnGroup 同時或錯峰出怪。
struct SpawnGroup {
    EnemyTypeId type = EnemyTypeId::Regular; // 該組敵人種類
    int count = 0; // 該組總數量
    float startDelay = 0.0F; // Wave 開始後幾秒才開始生這一組
    float interval = 1.0F; // 同組每隻之間的出生間隔（秒）
    std::vector<int> spawnPointIndices; // 指定起點索引；空陣列 = 所有起點
};

// -------------------- 單波配置 --------------------
// 說明：一個關卡由多個 WaveConfig 組成。
struct WaveConfig {
    int waveNumber = 1; // 波次編號（給 UI / 除錯）
    bool isBossWave = false; // 是否為 Boss 波
    float prepTime = 3.0F; // 進入此波前的準備時間（秒）
    int clearRewardGold = 0; // 清完此波的金幣獎勵
    std::vector<SpawnGroup> groups; // 此波包含的所有生怪組
};

// -------------------- 單關配置 --------------------
// 說明：POC 階段先用靜態資料表，後續可平滑改為讀檔。
struct LevelConfig {
    int levelNumber = 1; // 關卡編號
    std::string mapPath; // 地圖檔路徑（例如 assets/maps/map_01.csv）
    int baseHp = 20; // 本關基地初始血量
    int startingGold = 120; // 本關初始金幣
    std::vector<WaveConfig> waves; // 本關全部波次
};

// -------------------- 前兩關靜態配置 --------------------
// 設計目標：
// 1) 第 1 關教 Regular / Fast / Strong
// 2) 第 2 關加入 Heli 做對空壓力檢查
inline const std::vector<LevelConfig> kLevelConfigs = {
    // -------------------- Level 1：教學關 --------------------
    {
        1,
        "assets/maps/map_01.csv",
        20,
        120,
        {
            // W1
            {
                1, false, 4.0F, 20,
                {
                    {EnemyTypeId::Regular, 8, 0.0F, 0.95F, {}}
                }
            },
            // W2
            {
                2, false, 4.0F, 25,
                {
                    {EnemyTypeId::Regular, 10, 0.0F, 0.80F, {}},
                    {EnemyTypeId::Fast, 4, 1.5F, 1.10F, {}}
                }
            },
            // W3
            {
                3, false, 4.0F, 30,
                {
                    {EnemyTypeId::Strong, 6, 0.0F, 1.20F, {}},
                    {EnemyTypeId::Regular, 8, 0.8F, 0.85F, {}}
                }
            },
            // W4
            {
                4, false, 4.0F, 35,
                {
                    {EnemyTypeId::Fast, 14, 0.0F, 0.50F, {}}
                }
            },
            // W5
            {
                5, false, 5.0F, 40,
                {
                    {EnemyTypeId::Strong, 8, 0.0F, 1.10F, {}},
                    {EnemyTypeId::Regular, 10, 0.8F, 0.75F, {}},
                    {EnemyTypeId::Fast, 6, 3.0F, 0.55F, {}}
                }
            }
        }
    },

    // -------------------- Level 2：對空檢查關 --------------------
    {
        2,
        "assets/maps/map_02.csv",
        20,
        160,
        {
            // W1
            {
                1, false, 4.0F, 25,
                {
                    {EnemyTypeId::Regular, 10, 0.0F, 0.80F, {}},
                    {EnemyTypeId::Fast, 6, 1.0F, 0.70F, {}}
                }
            },
            // W2
            {
                2, false, 4.0F, 30,
                {
                    {EnemyTypeId::Heli, 4, 0.0F, 1.60F, {}},
                    {EnemyTypeId::Regular, 10, 0.5F, 0.80F, {}}
                }
            },
            // W3
            {
                3, false, 4.0F, 35,
                {
                    {EnemyTypeId::Heli, 6, 0.0F, 1.20F, {}},
                    {EnemyTypeId::Fast, 8, 1.0F, 0.65F, {}}
                }
            },
            // W4
            {
                4, false, 5.0F, 40,
                {
                    {EnemyTypeId::Strong, 6, 0.0F, 1.10F, {}},
                    {EnemyTypeId::Heli, 4, 2.0F, 1.35F, {}},
                    {EnemyTypeId::Regular, 8, 0.5F, 0.75F, {}}
                }
            },
            // W5
            {
                5, false, 5.0F, 50,
                {
                    {EnemyTypeId::Heli, 6, 0.0F, 1.10F, {}},
                    {EnemyTypeId::Strong, 6, 2.0F, 1.00F, {}},
                    {EnemyTypeId::Fast, 6, 0.5F, 0.65F, {}}
                }
            }
        }
    }
};

// -------------------- 關卡查表介面 --------------------
inline void validateLevelConfigs() {
    std::vector<std::string> errors;
    std::unordered_set<int> seenLevelNumbers;

    for (const LevelConfig& levelConfig : kLevelConfigs) {
        if (!seenLevelNumbers.insert(levelConfig.levelNumber).second) {
            errors.push_back("重複關卡編號: " + std::to_string(levelConfig.levelNumber));
        }
        if (levelConfig.levelNumber <= 0) {
            errors.push_back("非法關卡編號: " + std::to_string(levelConfig.levelNumber));
        }
        if (levelConfig.baseHp <= 0) {
            errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) + " 的 baseHp 必須大於 0");
        }
        if (levelConfig.mapPath.empty()) {
            errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) + " 缺少 mapPath");
        } else if (!std::filesystem::exists(levelConfig.mapPath)) {
            errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) + " 地圖不存在: " + levelConfig.mapPath);
        }
        if (levelConfig.waves.empty()) {
            errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) + " 沒有任何 wave");
        }

        for (const WaveConfig& waveConfig : levelConfig.waves) {
            if (waveConfig.waveNumber <= 0) {
                errors.push_back(
                    "關卡 " + std::to_string(levelConfig.levelNumber) +
                    " 出現非法 waveNumber: " + std::to_string(waveConfig.waveNumber)
                );
            }
            if (waveConfig.prepTime < 0.0F) {
                errors.push_back(
                    "關卡 " + std::to_string(levelConfig.levelNumber) +
                    " wave " + std::to_string(waveConfig.waveNumber) + " prepTime 不可小於 0"
                );
            }
            if (waveConfig.groups.empty()) {
                errors.push_back(
                    "關卡 " + std::to_string(levelConfig.levelNumber) +
                    " wave " + std::to_string(waveConfig.waveNumber) + " 沒有 spawn group"
                );
            }

            for (const SpawnGroup& spawnGroup : waveConfig.groups) {
                if (spawnGroup.count <= 0) {
                    errors.push_back(
                        "關卡 " + std::to_string(levelConfig.levelNumber) +
                        " wave " + std::to_string(waveConfig.waveNumber) + " group count 必須大於 0"
                    );
                }
                if (spawnGroup.startDelay < 0.0F) {
                    errors.push_back(
                        "關卡 " + std::to_string(levelConfig.levelNumber) +
                        " wave " + std::to_string(waveConfig.waveNumber) + " group startDelay 不可小於 0"
                    );
                }
                if (spawnGroup.interval < 0.0F) {
                    errors.push_back(
                        "關卡 " + std::to_string(levelConfig.levelNumber) +
                        " wave " + std::to_string(waveConfig.waveNumber) + " group interval 不可小於 0"
                    );
                }
            }
        }
    }

    if (!errors.empty()) {
        std::ostringstream oss;
        oss << "LevelConfig 驗證失敗(" << errors.size() << "):";
        for (const std::string& error : errors) {
            oss << "\n - " << error;
        }
        throw std::runtime_error(oss.str());
    }
}

inline const std::vector<LevelConfig>& getAllLevelConfigs() {
    static const bool kValidated = []() {
        validateLevelConfigs();
        return true;
    }();
    (void)kValidated;
    return kLevelConfigs;
}

inline const LevelConfig& getLevelConfig(int levelNumber) {
    static const bool kValidated = []() {
        validateLevelConfigs();
        return true;
    }();
    (void)kValidated;

    for (const LevelConfig& levelConfig : kLevelConfigs) {
        if (levelConfig.levelNumber == levelNumber) {
            return levelConfig;
        }
    }
    throw std::runtime_error("找不到指定關卡配置");
}

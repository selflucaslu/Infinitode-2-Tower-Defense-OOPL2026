#pragma once

#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "enemy/EnemyTypeConfig.hpp"

// -------------------- 刷怪群組配置 --------------------
// 代表某一波（Wave）內，某一種特定類型敵人的生成規則
struct SpawnGroup {
  EnemyTypeId type = EnemyTypeId::Regular; // 生成的敵人類型
  int count = 0;                           // 該組生成的敵人數量
  float startDelay = 0.0F;                 // 波次倒計時結束後，第一隻敵人的延遲生成時間（秒）
  float interval = 1.0F;                   // 敵人間隔生成時間（秒）
  std::vector<int> spawnPointIndices;      // 生成起點的索引列表（為空時在所有起點生成）
  
  // 循環強化乘數（預設 1.0，無限循環模式中會依輪數乘算）
  float hpMultiplier  = 1.0F;              // 血量強化倍率（例如 1.2）
  float spdMultiplier = 1.0F;              // 速度強化倍率（例如 1.1）
};

// -------------------- 波次配置 --------------------
// 代表單一關卡中的某個波次，控制倒數計時與刷怪細節
struct WaveConfig {
  int waveNumber = 1;             // 波次編號（用於 UI / 統計）
  bool isBossWave = false;        // 是否為 Boss 波次
  float prepTime = 3.0F;          // 該波開始前的準備倒數計時（秒）
  int clearRewardGold = 0;        // 清空此波怪物後的獎勵金幣
  std::vector<SpawnGroup> groups; // 該波包含的所有刷怪群組
};

// -------------------- 關卡配置 --------------------
// 儲存關卡的整體基本數值與所屬波次列表
struct LevelConfig {
  int levelNumber = 1;           // 關卡編號
  std::string mapPath;           // 地圖 CSV 檔案路徑（如 assets/maps/map_01.csv）
  int baseHp = 20;               // 基地初始生命值
  int startingGold = 150;        // 初始金幣
  std::vector<WaveConfig> waves; // 關卡的所有波次列表
};

// -------------------- 全域關卡資料宣告 --------------------
// 於 src/game/LevelConfig.cpp 實作，作為全域關卡配置表
extern const std::vector<LevelConfig> kLevelConfigs;


// -------------------- 關卡配置校驗 --------------------
inline void validateLevelConfigs() {
  std::vector<std::string> errors;
  std::unordered_set<int> seenLevelNumbers;

  for (const LevelConfig &levelConfig : kLevelConfigs) {
    if (!seenLevelNumbers.insert(levelConfig.levelNumber).second) {
      errors.push_back("重複的關卡編號: " +
                       std::to_string(levelConfig.levelNumber));
    }
    if (levelConfig.levelNumber <= 0) {
      errors.push_back("無效的關卡編號: " +
                       std::to_string(levelConfig.levelNumber));
    }
    if (levelConfig.baseHp <= 0) {
      errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                       " 的 baseHp 必須大於 0");
    }
    if (levelConfig.mapPath.empty()) {
      errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                       " 未設定 mapPath");
    } else if (!std::filesystem::exists(levelConfig.mapPath)) {
      errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                       " 的地圖檔案不存在: " + levelConfig.mapPath);
    }
    if (levelConfig.waves.empty()) {
      errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                       " 沒有配置任何 wave");
    }

    for (const WaveConfig &waveConfig : levelConfig.waves) {
      if (waveConfig.waveNumber <= 0) {
        errors.push_back(
            "關卡 " + std::to_string(levelConfig.levelNumber) +
            " 的無效 waveNumber: " + std::to_string(waveConfig.waveNumber));
      }
      if (waveConfig.prepTime < 0.0F) {
        errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                         " wave " + std::to_string(waveConfig.waveNumber) +
                         " 的 prepTime 不能小於 0");
      }
      if (waveConfig.groups.empty()) {
        errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                         " wave " + std::to_string(waveConfig.waveNumber) +
                         " 沒有配置任何 spawn group");
      }

      for (const SpawnGroup &spawnGroup : waveConfig.groups) {
        if (spawnGroup.count <= 0) {
          errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                           " wave " + std::to_string(waveConfig.waveNumber) +
                           " 的 group count 必須大於 0");
        }
        if (spawnGroup.startDelay < 0.0F) {
          errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                           " wave " + std::to_string(waveConfig.waveNumber) +
                           " 的 group startDelay 不能小於 0");
        }
        if (spawnGroup.interval < 0.0F) {
          errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                           " wave " + std::to_string(waveConfig.waveNumber) +
                           " 的 group interval 不能小於 0");
        }
      }
    }
  }

  if (!errors.empty()) {
    std::ostringstream oss;
    oss << "LevelConfig validation failed (" << errors.size() << "):";
    for (const std::string &error : errors) {
      oss << "\n - " << error;
    }
    throw std::runtime_error(oss.str());
  }
}

inline const std::vector<LevelConfig> &getAllLevelConfigs() {
  static const bool kValidated = []() {
    validateLevelConfigs();
    return true;
  }();
  (void)kValidated;
  return kLevelConfigs;
}

inline const LevelConfig &getLevelConfig(int levelNumber) {
  static const bool kValidated = []() {
    validateLevelConfigs();
    return true;
  }();
  (void)kValidated;

  for (const LevelConfig &levelConfig : kLevelConfigs) {
    if (levelConfig.levelNumber == levelNumber) {
      return levelConfig;
    }
  }
  throw std::runtime_error("Level config not found");
}

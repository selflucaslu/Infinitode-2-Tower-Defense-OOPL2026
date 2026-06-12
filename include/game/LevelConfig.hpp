#pragma once

#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "enemy/EnemyTypeConfig.hpp"

// -------------------- ?獢??賹????--------------------
// ?方??垢???Wave ??察?????SpawnGroup ????謖??????氐?
struct SpawnGroup {
  EnemyTypeId type = EnemyTypeId::Regular; // ?啗???????
  int count = 0;                           // ?啗??株都???
  float startDelay = 0.0F;                 // Wave ????綽?曇???????賹謕???
  float interval = 1.0F;                   // ????伍謍??????賹???????
  std::vector<int> spawnPointIndices;      // ??????撥???敺??? = ????衣結??
  // ????箄????????????1.0 = ?????蹓??????
  float hpMultiplier  = 1.0F;              // ?肅??授??????箄???1.2??
  float spdMultiplier = 1.0F;              // ????賹撞???????箄???1.1??
};

// -------------------- ??潮?? --------------------
// ?方??垢????????叟城??WaveConfig ?荔????
struct WaveConfig {
  int waveNumber = 1;             // ?撓瘣餉????蝑?UI / ??伐???
  bool isBossWave = false;        // ??秋???Boss ??
  float prepTime = 3.0F;          // ??????????□??蹇??????
  int clearRewardGold = 0;        // ?????????????
  std::vector<SpawnGroup> groups; // ?????荒??????????
};

// -------------------- ?獢??? --------------------
// ?方??忝OC ??????謚????萄???綽???砍????蝞??澗???
struct LevelConfig {
  int levelNumber = 1;           // ?謚迎??箏?
  std::string mapPath;           // ????澗?璆????? assets/maps/map_01.csv??
  int baseHp = 20;               // ?蟡??蝞秘?豲??肅??
  int startingGold = 150;        // ?蟡??豲???恍有
  std::vector<WaveConfig> waves; // ?蟡???賂貊?嚗豢暑
};

// -------------------- ?謚??謚迎?????? --------------------
// ???城蟡謚砍???src/game/LevelConfig.cpp?蹓?????TU ?????銋?豲???
//       ?蹎??箏???OOM ?????
extern const std::vector<LevelConfig> kLevelConfigs;


// -------------------- ?謚迎??鈭乒??∵? --------------------
inline void validateLevelConfigs() {
  std::vector<std::string> errors;
  std::unordered_set<int> seenLevelNumbers;

  for (const LevelConfig &levelConfig : kLevelConfigs) {
    if (!seenLevelNumbers.insert(levelConfig.levelNumber).second) {
      errors.push_back("????謚迎??箏?: " +
                       std::to_string(levelConfig.levelNumber));
    }
    if (levelConfig.levelNumber <= 0) {
      errors.push_back("?豯??謚迎??箏?: " +
                       std::to_string(levelConfig.levelNumber));
    }
    if (levelConfig.baseHp <= 0) {
      errors.push_back("?謚迎? " + std::to_string(levelConfig.levelNumber) +
                       " ??baseHp ?對??剜謘?0");
    }
    if (levelConfig.mapPath.empty()) {
      errors.push_back("?謚迎? " + std::to_string(levelConfig.levelNumber) +
                       " ?餌?? mapPath");
    } else if (!std::filesystem::exists(levelConfig.mapPath)) {
      errors.push_back("?謚迎? " + std::to_string(levelConfig.levelNumber) +
                       " ???????? " + levelConfig.mapPath);
    }
    if (levelConfig.waves.empty()) {
      errors.push_back("?謚迎? " + std::to_string(levelConfig.levelNumber) +
                       " ????２? wave");
    }

    for (const WaveConfig &waveConfig : levelConfig.waves) {
      if (waveConfig.waveNumber <= 0) {
        errors.push_back(
            "?謚迎? " + std::to_string(levelConfig.levelNumber) +
            " ?蝞??豯? waveNumber: " + std::to_string(waveConfig.waveNumber));
      }
      if (waveConfig.prepTime < 0.0F) {
        errors.push_back("?謚迎? " + std::to_string(levelConfig.levelNumber) +
                         " wave " + std::to_string(waveConfig.waveNumber) +
                         " prepTime ???餅??船? 0");
      }
      if (waveConfig.groups.empty()) {
        errors.push_back("?謚迎? " + std::to_string(levelConfig.levelNumber) +
                         " wave " + std::to_string(waveConfig.waveNumber) +
                         " ??? spawn group");
      }

      for (const SpawnGroup &spawnGroup : waveConfig.groups) {
        if (spawnGroup.count <= 0) {
          errors.push_back("?謚迎? " + std::to_string(levelConfig.levelNumber) +
                           " wave " + std::to_string(waveConfig.waveNumber) +
                           " group count ?對??剜謘?0");
        }
        if (spawnGroup.startDelay < 0.0F) {
          errors.push_back("?謚迎? " + std::to_string(levelConfig.levelNumber) +
                           " wave " + std::to_string(waveConfig.waveNumber) +
                           " group startDelay ???餅??船? 0");
        }
        if (spawnGroup.interval < 0.0F) {
          errors.push_back("?謚迎? " + std::to_string(levelConfig.levelNumber) +
                           " wave " + std::to_string(waveConfig.waveNumber) +
                           " group interval ???餅??船? 0");
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

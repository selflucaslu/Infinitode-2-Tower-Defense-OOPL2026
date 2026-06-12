#pragma once

#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include "enemy/EnemyTypeConfig.hpp"

// -------------------- 單組生怪配置 --------------------
// 說明：一個 Wave 內可有多個 SpawnGroup 同時或錯峰出怪。
struct SpawnGroup {
  EnemyTypeId type = EnemyTypeId::Regular; // 該組敵人種類
  int count = 0;                           // 該組總數量
  float startDelay = 0.0F;                 // Wave 開始後幾秒才開始生這一組
  float interval = 1.0F;                   // 同組每隻之間的出生間隔（秒）
  std::vector<int> spawnPointIndices;      // 指定起點索引；空陣列 = 所有起點
  // 無限循環強化倍率（預設 1.0 = 第一輪、不強化）
  float hpMultiplier  = 1.0F;              // 血量倍率（每循環 ×1.2）
  float spdMultiplier = 1.0F;              // 移動速度倍率（每循環 ×1.1）
};

// -------------------- 單波配置 --------------------
// 說明：一個關卡由多個 WaveConfig 組成。
struct WaveConfig {
  int waveNumber = 1;             // 波次編號（給 UI / 除錯）
  bool isBossWave = false;        // 是否為 Boss 波
  float prepTime = 3.0F;          // 進入此波前的準備時間（秒）
  int clearRewardGold = 0;        // 清完此波的金幣獎勵
  std::vector<SpawnGroup> groups; // 此波包含的所有生怪組
};

// -------------------- 單關配置 --------------------
// 說明：POC 階段先用靜態資料表，後續可平滑改為讀檔。
struct LevelConfig {
  int levelNumber = 1;           // 關卡編號
  std::string mapPath;           // 地圖檔路徑（例如 assets/maps/map_01.csv）
  int baseHp = 20;               // 本關基地初始血量
  int startingGold = 150;        // 本關初始金幣
  std::vector<WaveConfig> waves; // 本關全部波次
};

// -------------------- 前兩關靜態配置 --------------------
// 設計目標：
// 1) 第 1 關教 Regular / Fast / Strong
// 2) 第 2 關加入 Heli 做對空壓力檢查
inline const std::vector<LevelConfig> kLevelConfigs = {
    // -------------------- Level 1：教學關 --------------------
    {1,
     "assets/maps/map_01.csv",
     20,
     120,
     {// W1
      {1, false, 4.0F, 20, {{EnemyTypeId::Regular, 8, 0.0F, 0.95F, {}}}},
      // W2
      {2,
       false,
       4.0F,
       25,
       {{EnemyTypeId::Regular, 10, 0.0F, 0.80F, {}},
        {EnemyTypeId::Fast, 4, 1.5F, 1.10F, {}}}},
      // W3
      {3,
       false,
       4.0F,
       30,
       {{EnemyTypeId::Strong, 6, 0.0F, 1.20F, {}},
        {EnemyTypeId::Regular, 8, 0.8F, 0.85F, {}}}}}},

    // -------------------- Level 2：對空檢查關 --------------------
    {2,
     "assets/maps/map_02.csv",
     20,
     160,
     {// W1
      {1,
       false,
       4.0F,
       25,
       {{EnemyTypeId::Regular, 10, 0.0F, 0.80F, {}},
        {EnemyTypeId::Fast, 6, 1.0F, 0.70F, {}}}},
      // W2
      {2,
       false,
       4.0F,
       30,
       {{EnemyTypeId::Heli, 4, 0.0F, 1.60F, {}},
        {EnemyTypeId::Regular, 10, 0.5F, 0.80F, {}}}},
      // W3
      {3,
       false,
       4.0F,
       35,
       {{EnemyTypeId::Heli, 6, 0.0F, 1.20F, {}},
        {EnemyTypeId::Fast, 8, 1.0F, 0.65F, {}}}},
      // W4
      {4,
       false,
       5.0F,
       40,
       {{EnemyTypeId::Strong, 6, 0.0F, 1.10F, {}},
        {EnemyTypeId::Heli, 4, 2.0F, 1.35F, {}},
        {EnemyTypeId::Regular, 8, 0.5F, 0.75F, {}}}},
      // W5
      {5,
       false,
       5.0F,
       50,
       {{EnemyTypeId::Heli, 6, 0.0F, 1.10F, {}},
        {EnemyTypeId::Strong, 6, 2.0F, 1.00F, {}},
        {EnemyTypeId::Fast, 6, 0.5F, 0.65F, {}}}}}

    },
    // -------------------- Level 3：飢腸轆轆的貪婪章魚鮑勃 --------------------
    {// 現在你手上只有200元
     3,
     "assets/maps/map_03.csv",
     1,
     200,
     {// W1
      {1,
       false,
       4.0F,
       30,
       {
           {EnemyTypeId::Armored, 4, 0.5F, 1.00F, {}},
           {EnemyTypeId::Regular, 4, 0.0F, 0.60F, {}},
           {EnemyTypeId::Strong, 4, 0.3F, 0.70F, {}},
       }},
      // W2
      {2,
       false,
       4.0F,
       30,
       {
           {EnemyTypeId::Strong, 10, 0.0F, 1.60F, {}},
           {EnemyTypeId::Fast, 10, 0.5F, 1.6F, {}},
           {EnemyTypeId::Heli, 6, 0.0F, 1.20F, {}},
           {EnemyTypeId::Regular, 8, 0.5F, 0.75F, {}},
           {EnemyTypeId::Armored, 6, 1.0F, 1.00F, {}},
       }},
      // W3
      {3,
       false,
       4.0F,
       35,
       {
           {EnemyTypeId::Strong, 20, 0.0F, 1.60F, {}},
           {EnemyTypeId::Fast, 20, 0.5F, 1.6F, {}},
           {EnemyTypeId::Heli, 20, 0.0F, 1.20F, {}},
           {EnemyTypeId::Regular, 20, 0.5F, 0.75F, {}},
           {EnemyTypeId::Armored, 20, 1.0F, 1.00F, {}},
       }},
      // W4
      {4,
       false,
       5.0F,
       40,
       {{EnemyTypeId::Armored, 100, 0.0F, 0.4F, {}},
        {EnemyTypeId::Strong, 100, 0.0F, 0.50F, {}},
        {EnemyTypeId::Fast, 100, 0.0F, 0.1F, {}},
        {EnemyTypeId::Heli, 100, 0.0F, 0.6F, {}},
        {EnemyTypeId::Regular, 100, 0.0F, 0.3F, {}},
        {EnemyTypeId::Jet, 100, 0.0F, 0.2F, {}}}},
        // W5
      {5,
       false,
       5.0F,
       40,
       {{EnemyTypeId::Armored, 100, 0.0F, 0.04F, {}},
        {EnemyTypeId::Strong, 100, 0.0F, 0.050F, {}},
        {EnemyTypeId::Fast, 100, 0.0F, 0.01F, {}},
        {EnemyTypeId::Heli, 100, 0.0F, 0.06F, {}},
        {EnemyTypeId::Regular, 100, 0.0F, 0.03F, {}},
        {EnemyTypeId::Jet, 100, 0.0F, 0.02F, {}}}},
      // W6
      {6,
       false,
       5.0F,
       50,
       {{EnemyTypeId::Armored,500, 0.0F, 0.05F, {}},
        {EnemyTypeId::Strong, 500, 0.0F, 0.050F, {}},
        {EnemyTypeId::Fast, 500, 0.0F, 0.005F, {}},
        {EnemyTypeId::Heli, 500, 0.0F, 0.06F, {}},
        {EnemyTypeId::Regular, 500, 0.0F, 0.04F, {}},
        {EnemyTypeId::Jet, 500, 0.0F, 0.025F, {}}}}}},
        // -------------------- Level 3：飢腸轆轆的貪婪章魚鮑勃 --------------------
    {// 現在你手上只有200元
     4,
     "assets/maps/map_04.csv",
     100,
     200,
     {// W1
      {1,
       false,
       4.0F,
       30,
       {
           {EnemyTypeId::Regular,10,0.00f,1.00f,{}},
           {EnemyTypeId::Icy,2,0.0f,1.0f,{}}
       }},
       {2,
       false,
       4.0F,
       30,
       {
           {EnemyTypeId::Regular,12,0.00f,1.00f,{}}
       }},
      {3,
       false,
       4.0F,
       30,
       {
           {EnemyTypeId::Armored,2,0.00f,1.00f,{}},
           {EnemyTypeId::Regular,5,0.00f,0.80f,{}},
           {EnemyTypeId::Armored,2,0.0f,1.00f,{}},
           {EnemyTypeId::Fast,3,0.0f,0.60f,{}},
           {EnemyTypeId::Healer,2,0.0f,0.60f,{}},
           {EnemyTypeId::Jet,2,0.0f,0.60f,{}}

      }},
      {
        4,
        false,
        4.0F,
        30,
        {
          {EnemyTypeId::Armored,10,0.0f,1.00f,{}},
          {EnemyTypeId::Strong,10,0.0f,1.00f,{}},
          {EnemyTypeId::Regular,15,0.0f,1.00f,{}},
          {EnemyTypeId::Fast,5,0.0f,1.00f,{}},
          {EnemyTypeId::Heli,10,0.0f,1.00f,{}},
          {EnemyTypeId::Jet,5,0.0f,1.00f,{}},
          {EnemyTypeId::Toxic,2,0.0f,1.0f,{}},
          {EnemyTypeId::Icy,2,0.0f,1.0f,{}}
        }
      },
      {
        5,
        false,
        4.0F,
        30,
        {
          {EnemyTypeId::Armored,10,0.0f,1.00f,{}},
          {EnemyTypeId::Strong,10,0.0f,1.00f,{}},
          {EnemyTypeId::Healer,10,0.0f,1.00f,{}},
          {EnemyTypeId::Fast,10,0.0f,1.00f,{}},
          {EnemyTypeId::Regular,15,0.0f,1.00f,{}},
          {EnemyTypeId::Jet,12,0.0f,1.0f,{}},
          {EnemyTypeId::Fighter,5,0.0f,1.0f,{}},
          {EnemyTypeId::Fast,5,0.0f,1.00f,{}},
          {EnemyTypeId::Heli,10,0.0f,1.00f,{}},
          {EnemyTypeId::Jet,5,0.0f,1.00f,{}},
          {EnemyTypeId::Toxic,5,0.0f,1.0f,{}},
          {EnemyTypeId::Icy,5,0.0f,1.0f,{}}
        }
      },
      

      
    }},
    {
      5,
      "assets/maps/map_05.csv",
      100,
      200,
      {
        {1,
         false,
         4.0F,
         30,
         {
             {EnemyTypeId::Regular,1,0.00f,1.00f,{}},
             {EnemyTypeId::Armored,1,0.00f,1.0f,{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99}},
             {EnemyTypeId::Healer,1,0.0f,1.0f,{25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74}}
         }},
         {
          2,
          false,
          4.0f,
          30,
          {
            {EnemyTypeId::Regular,100,0.0f,1.00f,{0}},
            {EnemyTypeId::Armored,100,0.0f,1.00f,{1}},
            {EnemyTypeId::Healer,100,0.0f,1.00f,{2}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{3}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{4}},
            {EnemyTypeId::Jet,100,0.0f,1.0f,{5}},
            {EnemyTypeId::Fighter,100,0.0f,1.0f,{6}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{7}},
            {EnemyTypeId::Heli,100,0.0f,1.00f,{8}},
            {EnemyTypeId::Jet,100,0.0f,1.00f,{9}},
            {EnemyTypeId::Toxic,100,0.0f,1.0f,{10}},
            {EnemyTypeId::Icy,100,0.0f,1.0f,{11}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{12}},
            {EnemyTypeId::Armored,100,0.0f,1.00f,{13}},
            {EnemyTypeId::Healer,100,0.0f,1.00f,{14}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{15}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{16}},
            {EnemyTypeId::Jet,100,0.0f,1.0f,{17}},
            {EnemyTypeId::Fighter,100,0.0f,1.0f,{18}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{19}},
            {EnemyTypeId::Heli,100,0.0f,1.00f,{20}},
            {EnemyTypeId::Jet,100,0.0f,1.00f,{21}},
            {EnemyTypeId::Toxic,100,0.0f,1.0f,{22}},
            {EnemyTypeId::Icy,100,0.0f,1.0f,{23}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{24}},
            {EnemyTypeId::Armored,100,0.0f,1.00f,{26}},
            {EnemyTypeId::Healer,100,0.0f,1.00f,{28}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{30}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{32}},
            {EnemyTypeId::Jet,100,0.0f,1.0f,{34}},
            {EnemyTypeId::Fighter,100,0.0f,1.0f,{36}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{38}},
            {EnemyTypeId::Heli,100,0.0f,1.00f,{40}},
            {EnemyTypeId::Jet,100,0.0f,1.00f,{42}},
            {EnemyTypeId::Toxic,100,0.0f,1.0f,{44}},
            {EnemyTypeId::Icy,100,0.0f,1.0f,{46}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{48}},
            {EnemyTypeId::Armored,100,0.0f,1.00f,{50}},
            {EnemyTypeId::Healer,100,0.0f,1.00f,{52}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{54}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{56}},
            {EnemyTypeId::Jet,100,0.0f,1.0f,{58}},
            {EnemyTypeId::Fighter,100,0.0f,1.0f,{60}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{62}},
            {EnemyTypeId::Heli,100,0.0f,1.00f,{64}},
            {EnemyTypeId::Jet,100,0.0f,1.00f,{66}},
            {EnemyTypeId::Toxic,100,0.0f,1.0f,{68}},
            {EnemyTypeId::Icy,100,0.0f,1.0f,{70}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{72}},
            {EnemyTypeId::Armored,100,0.0f,1.00f,{74}},
            {EnemyTypeId::Healer,100,0.0f,1.00f,{99}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{98}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{97}},
            {EnemyTypeId::Jet,100,0.0f,1.0f,{96}},
            {EnemyTypeId::Fighter,100,0.0f,1.0f,{95}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{94}},
            {EnemyTypeId::Heli,100,0.0f,1.00f,{93}},
            {EnemyTypeId::Jet,100,0.0f,1.00f,{92}},
            {EnemyTypeId::Toxic,100,0.0f,1.0f,{91}},
            {EnemyTypeId::Icy,100,0.0f,1.0f,{90}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{89}},
            {EnemyTypeId::Armored,100,0.0f,1.00f,{88}},
            {EnemyTypeId::Healer,100,0.0f,1.00f,{87}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{86}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{85}},
            {EnemyTypeId::Jet,100,0.0f,1.0f,{84}},
            {EnemyTypeId::Fighter,100,0.0f,1.0f,{83}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{82}},
            {EnemyTypeId::Heli,100,0.0f,1.00f,{81}},
            {EnemyTypeId::Jet,100,0.0f,1.00f,{80}},
            {EnemyTypeId::Toxic,100,0.0f,1.0f,{79}},
            {EnemyTypeId::Icy,100,0.0f,1.0f,{78}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{77}},
            {EnemyTypeId::Armored,100,0.0f,1.00f,{76}},
            {EnemyTypeId::Healer,100,0.0f,1.00f,{75}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{73}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{71}},
            {EnemyTypeId::Jet,100,0.0f,1.0f,{69}},
            {EnemyTypeId::Fighter,100,0.0f,1.0f,{67}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{65}},
            {EnemyTypeId::Heli,100,0.0f,1.00f,{63}},
            {EnemyTypeId::Jet,100,0.0f,1.00f,{61}},
            {EnemyTypeId::Toxic,100,0.0f,1.0f,{59}},
            {EnemyTypeId::Icy,100,0.0f,1.0f,{57}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{55}},
            {EnemyTypeId::Armored,100,0.0f,1.00f,{53}},
            {EnemyTypeId::Healer,100,0.0f,1.00f,{51}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{49}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{47}},
            {EnemyTypeId::Jet,100,0.0f,1.0f,{45}},
            {EnemyTypeId::Fighter,100,0.0f,1.0f,{43}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{41}},
            {EnemyTypeId::Heli,100,0.0f,1.00f,{39}},
            {EnemyTypeId::Jet,100,0.0f,1.00f,{37}},
            {EnemyTypeId::Toxic,100,0.0f,1.0f,{35}},
            {EnemyTypeId::Icy,100,0.0f,1.0f,{33}},
            {EnemyTypeId::Regular,100,0.0f,1.00f,{31}},
            {EnemyTypeId::Armored,100,0.0f,1.00f,{29}},
            {EnemyTypeId::Healer,100,0.0f,1.00f,{27}},
            {EnemyTypeId::Fast,100,0.0f,1.00f,{25}},
          }
         }
      }
    }

};

// -------------------- 關卡查表介面 --------------------
inline void validateLevelConfigs() {
  std::vector<std::string> errors;
  std::unordered_set<int> seenLevelNumbers;

  for (const LevelConfig &levelConfig : kLevelConfigs) {
    if (!seenLevelNumbers.insert(levelConfig.levelNumber).second) {
      errors.push_back("重複關卡編號: " +
                       std::to_string(levelConfig.levelNumber));
    }
    if (levelConfig.levelNumber <= 0) {
      errors.push_back("非法關卡編號: " +
                       std::to_string(levelConfig.levelNumber));
    }
    if (levelConfig.baseHp <= 0) {
      errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                       " 的 baseHp 必須大於 0");
    }
    if (levelConfig.mapPath.empty()) {
      errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                       " 缺少 mapPath");
    } else if (!std::filesystem::exists(levelConfig.mapPath)) {
      errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                       " 地圖不存在: " + levelConfig.mapPath);
    }
    if (levelConfig.waves.empty()) {
      errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                       " 沒有任何 wave");
    }

    for (const WaveConfig &waveConfig : levelConfig.waves) {
      if (waveConfig.waveNumber <= 0) {
        errors.push_back(
            "關卡 " + std::to_string(levelConfig.levelNumber) +
            " 出現非法 waveNumber: " + std::to_string(waveConfig.waveNumber));
      }
      if (waveConfig.prepTime < 0.0F) {
        errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                         " wave " + std::to_string(waveConfig.waveNumber) +
                         " prepTime 不可小於 0");
      }
      if (waveConfig.groups.empty()) {
        errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                         " wave " + std::to_string(waveConfig.waveNumber) +
                         " 沒有 spawn group");
      }

      for (const SpawnGroup &spawnGroup : waveConfig.groups) {
        if (spawnGroup.count <= 0) {
          errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                           " wave " + std::to_string(waveConfig.waveNumber) +
                           " group count 必須大於 0");
        }
        if (spawnGroup.startDelay < 0.0F) {
          errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                           " wave " + std::to_string(waveConfig.waveNumber) +
                           " group startDelay 不可小於 0");
        }
        if (spawnGroup.interval < 0.0F) {
          errors.push_back("關卡 " + std::to_string(levelConfig.levelNumber) +
                           " wave " + std::to_string(waveConfig.waveNumber) +
                           " group interval 不可小於 0");
        }
      }
    }
  }

  if (!errors.empty()) {
    std::ostringstream oss;
    oss << "LevelConfig 驗證失敗(" << errors.size() << "):";
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
  throw std::runtime_error("找不到指定關卡配置");
}

#include "game/LevelConfig.hpp"

// We can define the entire vector programmatically.
static std::vector<LevelConfig> buildLevelConfigs() {
    std::vector<LevelConfig> configs;

    // --- Level 1 ---
    {
        LevelConfig lvl;
        lvl.levelNumber = 1;
        lvl.mapPath = "assets/maps/map_01.csv";
        lvl.baseHp = 20;
        lvl.startingGold = 120;
        lvl.waves = {
            {1, false, 4.0F, 20, {{EnemyTypeId::Regular, 8, 0.0F, 0.95F, {}}}},
            {2, false, 4.0F, 25, {{EnemyTypeId::Regular, 10, 0.0F, 0.80F, {}}, {EnemyTypeId::Fast, 4, 1.5F, 1.10F, {}}}},
            {3, false, 4.0F, 30, {{EnemyTypeId::Strong, 6, 0.0F, 1.20F, {}}, {EnemyTypeId::Regular, 8, 0.8F, 0.85F, {}}}}
        };
        configs.push_back(lvl);
    }

    // --- Level 2 ---
    {
        LevelConfig lvl;
        lvl.levelNumber = 2;
        lvl.mapPath = "assets/maps/map_02.csv";
        lvl.baseHp = 20;
        lvl.startingGold = 160;
        lvl.waves = {
            {1, false, 4.0F, 25, {{EnemyTypeId::Regular, 10, 0.0F, 0.80F, {}}, {EnemyTypeId::Fast, 6, 1.0F, 0.70F, {}}}},
            {2, false, 4.0F, 30, {{EnemyTypeId::Heli, 4, 0.0F, 1.60F, {}}, {EnemyTypeId::Regular, 10, 0.5F, 0.80F, {}}}},
            {3, false, 4.0F, 35, {{EnemyTypeId::Heli, 6, 0.0F, 1.20F, {}}, {EnemyTypeId::Fast, 8, 1.0F, 0.65F, {}}}},
            {4, false, 5.0F, 40, {{EnemyTypeId::Strong, 6, 0.0F, 1.10F, {}}, {EnemyTypeId::Heli, 4, 2.0F, 1.35F, {}}, {EnemyTypeId::Fast, 10, 1.0F, 0.65F, {}}}},
            {5, false, 5.0F, 50, {{EnemyTypeId::Heli, 6, 0.0F, 1.10F, {}}, {EnemyTypeId::Strong, 6, 2.0F, 1.00F, {}}, {EnemyTypeId::Fast, 6, 0.5F, 0.65F, {}}}}
        };
        configs.push_back(lvl);
    }

    // --- Level 3 ---
    {
        LevelConfig lvl;
        lvl.levelNumber = 3;
        lvl.mapPath = "assets/maps/map_03.csv";
        lvl.baseHp = 1;
        lvl.startingGold = 200;
        lvl.waves = {
            {1, false, 4.0F, 30, {{EnemyTypeId::Armored, 4, 0.5F, 1.00F, {}}, {EnemyTypeId::Regular, 4, 0.0F, 0.60F, {}}, {EnemyTypeId::Strong, 4, 0.3F, 0.70F, {}}}},
            {2, false, 4.0F, 30, {{EnemyTypeId::Strong, 10, 0.0F, 1.60F, {}}, {EnemyTypeId::Fast, 10, 0.5F, 1.6F, {}}, {EnemyTypeId::Heli, 6, 0.0F, 1.20F, {}}, {EnemyTypeId::Regular, 8, 0.5F, 0.75F, {}}, {EnemyTypeId::Armored, 6, 1.0F, 1.00F, {}}}},
            {3, false, 4.0F, 35, {{EnemyTypeId::Strong, 20, 0.0F, 1.60F, {}}, {EnemyTypeId::Fast, 20, 0.5F, 1.6F, {}}, {EnemyTypeId::Heli, 20, 0.0F, 1.20F, {}}, {EnemyTypeId::Regular, 20, 0.5F, 0.75F, {}}, {EnemyTypeId::Armored, 20, 1.0F, 1.00F, {}}}},
            {4, false, 5.0F, 40, {{EnemyTypeId::Armored, 100, 0.0F, 0.4F, {}}, {EnemyTypeId::Strong, 100, 0.0F, 0.50F, {}}, {EnemyTypeId::Fast, 100, 0.0F, 0.1F, {}}, {EnemyTypeId::Heli, 100, 0.0F, 0.6F, {}}, {EnemyTypeId::Regular, 100, 0.0F, 0.3F, {}}, {EnemyTypeId::Jet, 100, 0.0F, 0.2F, {}}}},
            {5, false, 5.0F, 40, {{EnemyTypeId::Armored, 100, 0.0F, 0.04F, {}}, {EnemyTypeId::Strong, 100, 0.0F, 0.050F, {}}, {EnemyTypeId::Fast, 100, 0.0F, 0.01F, {}}, {EnemyTypeId::Heli, 100, 0.0F, 0.06F, {}}, {EnemyTypeId::Regular, 100, 0.0F, 0.03F, {}}, {EnemyTypeId::Jet, 100, 0.0F, 0.02F, {}}}},
            {6, false, 5.0F, 50, {{EnemyTypeId::Armored, 500, 0.0F, 0.05F, {}}, {EnemyTypeId::Strong, 500, 0.0F, 0.050F, {}}, {EnemyTypeId::Fast, 500, 0.0F, 0.005F, {}}, {EnemyTypeId::Heli, 500, 0.0F, 0.06F, {}}, {EnemyTypeId::Regular, 500, 0.0F, 0.04F, {}}, {EnemyTypeId::Jet, 500, 0.0F, 0.025F, {}}}}
        };
        configs.push_back(lvl);
    }

    // --- Level 4 ---
    {
        LevelConfig lvl;
        lvl.levelNumber = 4;
        lvl.mapPath = "assets/maps/map_04.csv";
        lvl.baseHp = 100;
        lvl.startingGold = 200;
        lvl.waves = {
            {1, false, 4.0F, 30, {{EnemyTypeId::Regular, 10, 0.00F, 1.00F, {}}, {EnemyTypeId::Icy, 2, 0.0F, 1.0F, {}}}},
            {2, false, 4.0F, 30, {{EnemyTypeId::Regular, 12, 0.00F, 1.00F, {}}}},
            {3, false, 4.0F, 30, {{EnemyTypeId::Armored, 2, 0.00F, 1.00F, {}}, {EnemyTypeId::Regular, 5, 0.00F, 0.80F, {}}, {EnemyTypeId::Armored, 2, 0.0F, 1.00F, {}}, {EnemyTypeId::Fast, 3, 0.0F, 0.60F, {}}, {EnemyTypeId::Healer, 2, 0.0F, 0.60F, {}}, {EnemyTypeId::Jet, 2, 0.0F, 0.60F, {}}}},
            {4, false, 4.0F, 30, {{EnemyTypeId::Armored, 10, 0.0F, 1.00F, {}}, {EnemyTypeId::Strong, 10, 0.0F, 1.00F, {}}, {EnemyTypeId::Regular, 15, 0.0F, 1.00F, {}}, {EnemyTypeId::Fast, 5, 0.0F, 1.00F, {}}, {EnemyTypeId::Heli, 10, 0.0F, 1.00F, {}}, {EnemyTypeId::Jet, 5, 0.0F, 1.00F, {}}, {EnemyTypeId::Toxic, 2, 0.0F, 1.0F, {}}, {EnemyTypeId::Icy, 2, 0.0F, 1.0F, {}}}},
            {5, false, 4.0F, 30, {{EnemyTypeId::Armored, 10, 0.0F, 1.00F, {}}, {EnemyTypeId::Strong, 10, 0.0F, 1.00F, {}}, {EnemyTypeId::Healer, 10, 0.0F, 1.00F, {}}, {EnemyTypeId::Fast, 10, 0.0F, 1.00F, {}}, {EnemyTypeId::Regular, 15, 0.0F, 1.00F, {}}, {EnemyTypeId::Jet, 12, 0.0F, 1.0F, {}}, {EnemyTypeId::Fighter, 5, 0.0F, 1.0F, {}}, {EnemyTypeId::Fast, 5, 0.0F, 1.00F, {}}, {EnemyTypeId::Heli, 10, 0.0F, 1.00F, {}}, {EnemyTypeId::Jet, 5, 0.0F, 1.00F, {}}, {EnemyTypeId::Toxic, 5, 0.0F, 1.0F, {}}, {EnemyTypeId::Icy, 5, 0.0F, 1.0F, {}}}}
        };
        configs.push_back(lvl);
    }

    // --- Level 5 ---
    {
        LevelConfig lvl;
        lvl.levelNumber = 5;
        lvl.mapPath = "assets/maps/map_05.csv";
        lvl.baseHp = 100;
        lvl.startingGold = 200;

        // Wave 1
        lvl.waves.push_back({1, false, 4.0F, 30, {
            {EnemyTypeId::Regular, 1, 0.00F, 1.00F, {}},
            {EnemyTypeId::Armored, 1, 0.00F, 1.0F, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99}},
            {EnemyTypeId::Healer, 1, 0.0F, 1.0F, {25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74}}
        }});

        // Wave 2: 300 groups generated programmatically
        {
            WaveConfig w2;
            w2.waveNumber = 2;
            w2.isBossWave = false;
            w2.prepTime = 4.0F;
            w2.clearRewardGold = 30;

            static const EnemyTypeId pattern[] = {
                EnemyTypeId::Regular,
                EnemyTypeId::Armored,
                EnemyTypeId::Healer,
                EnemyTypeId::Fast,
                EnemyTypeId::Regular,
                EnemyTypeId::Jet,
                EnemyTypeId::Fighter,
                EnemyTypeId::Fast,
                EnemyTypeId::Heli,
                EnemyTypeId::Jet,
                EnemyTypeId::Toxic,
                EnemyTypeId::Icy
            };

            for (int i = 0; i < 300; ++i) {
                SpawnGroup g;
                g.type = pattern[i % 12];
                g.count = 1;
                g.startDelay = i * 0.1f;
                g.interval = 1.0f;

                int subStep = i % 100;
                int spIdx = 0;
                if (subStep < 25) {
                    spIdx = subStep;
                } else if (subStep < 50) {
                    spIdx = 26 + (subStep - 25) * 2;
                } else if (subStep < 75) {
                    spIdx = 99 - (subStep - 50);
                } else {
                    spIdx = 73 - (subStep - 75) * 2;
                }
                g.spawnPointIndices = { spIdx };
                w2.groups.push_back(g);
            }
            lvl.waves.push_back(w2);
        }

        // Wave 3: 200 groups generated programmatically
        {
            WaveConfig w3;
            w3.waveNumber = 3;
            w3.isBossWave = false;
            w3.prepTime = 30.0F;
            w3.clearRewardGold = 30;

            static const EnemyTypeId startPattern[] = {
                EnemyTypeId::Armored,
                EnemyTypeId::Fast,
                EnemyTypeId::Fighter,
                EnemyTypeId::Fighter,
                EnemyTypeId::Fast,
                EnemyTypeId::Regular,
                EnemyTypeId::Heli,
                EnemyTypeId::Jet,
                EnemyTypeId::Toxic,
                EnemyTypeId::Icy,
                EnemyTypeId::Regular,
                EnemyTypeId::Armored
            };

            static const EnemyTypeId midPattern[] = {
                EnemyTypeId::Healer,
                EnemyTypeId::Fast,
                EnemyTypeId::Regular,
                EnemyTypeId::Jet,
                EnemyTypeId::Fighter,
                EnemyTypeId::Fast,
                EnemyTypeId::Heli,
                EnemyTypeId::Jet,
                EnemyTypeId::Toxic,
                EnemyTypeId::Icy,
                EnemyTypeId::Regular,
                EnemyTypeId::Armored
            };

            static const EnemyTypeId block50to65[] = {
                EnemyTypeId::Regular,
                EnemyTypeId::Regular,
                EnemyTypeId::Light,
                EnemyTypeId::Light,
                EnemyTypeId::Fighter,
                EnemyTypeId::Light,
                EnemyTypeId::Fighter,
                EnemyTypeId::Heli,
                EnemyTypeId::Fighter,
                EnemyTypeId::Fighter,
                EnemyTypeId::Fighter,
                EnemyTypeId::Healer,
                EnemyTypeId::Regular,
                EnemyTypeId::Fighter,
                EnemyTypeId::Heli,
                EnemyTypeId::Fighter
            };

            for (int i = 0; i < 200; ++i) {
                SpawnGroup g;
                g.count = 1;
                g.startDelay = i * 0.1f;
                g.interval = 1.0F;

                // Determine enemy type
                if (i < 12) {
                    g.type = startPattern[i];
                } else if (i < 50) {
                    g.type = midPattern[(i - 12) % 12];
                } else if (i < 66) {
                    g.type = block50to65[i - 50];
                } else {
                    g.type = EnemyTypeId::Fighter;
                }

                // Determine spawn point indices
                int spIdx = 0;
                if (i < 25) {
                    spIdx = 99 - i;
                } else if (i < 50) {
                    spIdx = 73 - (i - 25) * 2;
                } else if (i < 75) {
                    spIdx = i - 50;
                } else if (i < 100) {
                    spIdx = 26 + (i - 75) * 2;
                } else {
                    spIdx = 99 - (i - 100);
                }
                g.spawnPointIndices = { spIdx };
                w3.groups.push_back(g);
            }
            lvl.waves.push_back(w3);
        }

        configs.push_back(lvl);
    }

    return configs;
}

const std::vector<LevelConfig> kLevelConfigs = buildLevelConfigs();

#include "game/LevelConfig.hpp"
#include "enemy/EnemyTypeConfig.hpp"

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
        lvl.waves.push_back({1, false, 1.0F, 30, {
            {EnemyTypeId::Regular, 1, 0.00F, 1.00F, {}},
            {EnemyTypeId::Armored, 1, 0.00F, 1.0F, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99}},
            {EnemyTypeId::Healer, 1, 0.0F, 1.0F, {25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74}}
        }});

        // Wave 2: 300 groups generated programmatically
        {
            WaveConfig w2;
            w2.waveNumber = 2;
            w2.isBossWave = false;
            w2.prepTime = 1.0F;
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
            w3.prepTime = 1.0F;
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

        {
            WaveConfig w4;
            w4.waveNumber = 4;
            w4.isBossWave = false;
            w4.prepTime = 1.0F;
            w4.clearRewardGold = 30;

            static const EnemyTypeId round1to100[] = {
                EnemyTypeId::Armored,
                EnemyTypeId::Fast,
                EnemyTypeId::Fighter,
                EnemyTypeId::Healer,
                EnemyTypeId::Heli,
                EnemyTypeId::Icy,
                EnemyTypeId::Jet,
                EnemyTypeId::Light,
                EnemyTypeId::Regular,
                EnemyTypeId::Strong,
                EnemyTypeId::Toxic,
                EnemyTypeId::Strong,
                EnemyTypeId::Regular,
                EnemyTypeId::Strong,
                EnemyTypeId::Toxic,
                EnemyTypeId::Strong,
                EnemyTypeId::Regular,
                EnemyTypeId::Light,
                EnemyTypeId::Jet,
                EnemyTypeId::Icy,
                EnemyTypeId::Heli,
                EnemyTypeId::Fighter,
                EnemyTypeId::Fast,
                EnemyTypeId::Armored
            };
            static const EnemyTypeId round101to200_H[] = {
                EnemyTypeId::Armored,
                EnemyTypeId::Fast,
                EnemyTypeId::Fighter,
                EnemyTypeId::Healer,
                EnemyTypeId::Heli,
                EnemyTypeId::Icy,
                EnemyTypeId::Jet,
                EnemyTypeId::Light,
                EnemyTypeId::Regular,
                EnemyTypeId::Strong,
                EnemyTypeId::Toxic
            };
            static const EnemyTypeId round101to200_L[] = {
                EnemyTypeId::Toxic,
                EnemyTypeId::Strong,
                EnemyTypeId::Regular,
                EnemyTypeId::Light,
                EnemyTypeId::Jet,
                EnemyTypeId::Icy,
                EnemyTypeId::Heli,
                EnemyTypeId::Fighter,
                EnemyTypeId::Fast,
                EnemyTypeId::Armored
            };
            static const EnemyTypeId round201to300_H[] = {
                EnemyTypeId::Fighter,
                EnemyTypeId::Healer,
                EnemyTypeId::Heli,
                EnemyTypeId::Icy,
                EnemyTypeId::Jet,
                EnemyTypeId::Light,
                EnemyTypeId::Regular,
                EnemyTypeId::Strong,
                EnemyTypeId::Toxic,
                EnemyTypeId::Strong,
                EnemyTypeId::Regular
            };
            static const EnemyTypeId round201to300_L[] = {
                EnemyTypeId::Strong,
                EnemyTypeId::Regular,
                EnemyTypeId::Light,
                EnemyTypeId::Jet,
                EnemyTypeId::Icy,
                EnemyTypeId::Heli,
                EnemyTypeId::Fighter,
                EnemyTypeId::Fast,
                EnemyTypeId::Armored,
                EnemyTypeId::Strong,
                EnemyTypeId::Regular
            };
            static const EnemyTypeId round301to400_H[] = {
                EnemyTypeId::Healer,
                EnemyTypeId::Heli,
                EnemyTypeId::Icy,
                EnemyTypeId::Jet,
                EnemyTypeId::Light,
                EnemyTypeId::Regular,
                EnemyTypeId::Strong,
                EnemyTypeId::Toxic,
                EnemyTypeId::Strong,
                EnemyTypeId::Regular,
                EnemyTypeId::Light
            };
            static const EnemyTypeId round301to400_L[] = {
                EnemyTypeId::Strong,
                EnemyTypeId::Regular,
                EnemyTypeId::Light,
                EnemyTypeId::Jet,
                EnemyTypeId::Icy,
                EnemyTypeId::Heli,
                EnemyTypeId::Fighter,
                EnemyTypeId::Fast,
                EnemyTypeId::Armored,
                EnemyTypeId::Strong,
                EnemyTypeId::Regular
            };
            static const EnemyTypeId round401to500_H[] = {
                EnemyTypeId::Heli,
                EnemyTypeId::Icy,
                EnemyTypeId::Jet,
                EnemyTypeId::Light,
                EnemyTypeId::Regular,
                EnemyTypeId::Strong,
                EnemyTypeId::Toxic,
                EnemyTypeId::Strong,
                EnemyTypeId::Regular,
                EnemyTypeId::Light,
                EnemyTypeId::Jet
            };
            static const EnemyTypeId round401to500_L[] = {
                EnemyTypeId::Regular,
                EnemyTypeId::Light,
                EnemyTypeId::Jet,
                EnemyTypeId::Icy,
                EnemyTypeId::Heli,
                EnemyTypeId::Fighter,
                EnemyTypeId::Fast,
                EnemyTypeId::Armored,
                EnemyTypeId::Strong,
                EnemyTypeId::Regular,
                EnemyTypeId::Light
            };
            for(int i=0;i<500;i++){
                if(i<100){
                    SpawnGroup g;
                    g.count=1;
                    g.type=round1to100[i];
                    g.startDelay=i*0.1f;
                    g.interval=1.0f;
                    
                    int SpIdx1=0,SpIdx2=0;
                    if(i<25){
                        SpIdx1=75+i;
                        SpIdx2=24-i;
                    }else if(i<50){
                        SpIdx1=74-(i-25)*2;
                        SpIdx2=25+(i-25)*2;
                    }else if(i<75){
                        SpIdx1=24-(i-50);
                        SpIdx2=75+(i-50);
                    }else if(i<100){
                        SpIdx1=25+(i-75)*2;
                        SpIdx2=74-(i-75)*2;
                    }
                    g.spawnPointIndices={SpIdx1,SpIdx2};
                    w4.groups.push_back(g);
                }else{
                    SpawnGroup g1,g2;
                    g1.count=1;
                    g2.count=1;
                    g1.startDelay=(i-100)*0.05f;
                    g2.startDelay=(i-100)*0.05f+0.05f;
                    g1.interval=0.1f;
                    g2.interval=0.1f;
                    int idx = (i-100)%11;
                    if(i<200){
                        g1.type=round101to200_H[idx];
                        g2.type=round101to200_L[idx];
                    }else if(i<300){
                        g1.type=round201to300_H[idx];
                        g2.type=round201to300_L[idx];
                    }else if(i<400){
                        g1.type=round301to400_H[idx];
                        g2.type=round301to400_L[idx];
                    }else if(i<500){
                        g1.type=round401to500_H[idx];
                        g2.type=round401to500_L[idx];
                    }
                    int spIdx1=0,spIdx2=0,spIdx3=0,spIdx4=0;
                    int spidx=i%100;
                    if(spidx<25){
                        spIdx1=75+spidx;
                        spIdx2=74-spidx*2;
                        spIdx3=24-spidx;
                        spIdx4=25+spidx*2;
                    }else if(spidx<50){
                        spIdx1=74-(spidx-25)*2;
                        spIdx2=24-(spidx-25);
                        spIdx3=25+(spidx-25)*2;
                        spIdx4=75+(spidx-25);
                    }else if(spidx<75){
                        spIdx1=24-(spidx-50);
                        spIdx2=25+(spidx-50)*2;
                        spIdx3=75+(spidx-50);
                        spIdx4=74-(spidx-50)*2;
                    }else if(spidx<100){
                        spIdx1=25+(spidx-75)*2;
                        spIdx2=75+(spidx-75);
                        spIdx3=74-(spidx-75)*2;
                        spIdx4=24-(spidx-75);
                    }
                    g1.spawnPointIndices={spIdx1,spIdx3};
                    g2.spawnPointIndices={spIdx2,spIdx4};
                    w4.groups.push_back(g1);
                    w4.groups.push_back(g2);
                }
            }     

            lvl.waves.push_back(w4);
        }

        {
            WaveConfig w5;
            w5.waveNumber=5;
            w5.isBossWave=false;
            w5.prepTime=1.0F;
            w5.clearRewardGold=50;
            static const EnemyTypeId right[]={EnemyTypeId::Armored,EnemyTypeId::Fast,EnemyTypeId::Fighter,EnemyTypeId::Heli,EnemyTypeId::Regular};
            static const EnemyTypeId left[]={EnemyTypeId::Jet,EnemyTypeId::Icy,EnemyTypeId::Regular,EnemyTypeId::Toxic,EnemyTypeId::Strong,EnemyTypeId::Light,EnemyTypeId::Healer};
            for(int i=0;i<350;i++){
                SpawnGroup g1,g2;
                g1.count=1;
                g2.count=1;
                g1.startDelay=i*0.1f;
                g2.startDelay=i*0.1f;
                g1.interval=0.1f;
                g2.interval=0.1f;
                int rindex=i%5;
                int lindex=i%7;
                if(i<350){
                    g1.type=right[rindex];
                    g2.type=left[lindex];
                }
                int spIdx1=0,spIdx2=0;
                int spidx=i%100;
                if(spidx<25){
                    spIdx1=25+spidx*2;
                    spIdx2=spidx;
                }else if(spidx<50){
                    spIdx1=75+(spidx-25);
                    spIdx2=26+(spidx-25)*2;
                }else if(spidx<75){
                    spIdx1=74-(spidx-50)*2;
                    spIdx2=99-(spidx-50);
                }else if(spidx<100){
                    spIdx1=24-(spidx-75);
                    spIdx2=73-(spidx-75)*2;
                }
                g1.spawnPointIndices={spIdx1};
                g2.spawnPointIndices={spIdx2};
                w5.groups.push_back(g1);
                w5.groups.push_back(g2);
            }
            lvl.waves.push_back(w5);
        }

        {
            WaveConfig w6;
            w6.waveNumber=6;
            w6.isBossWave=false;
            w6.prepTime=1.0F;
            w6.clearRewardGold=50;
            static const EnemyTypeId left_car1[]={EnemyTypeId::Armored,EnemyTypeId::Fighter,EnemyTypeId::Icy,EnemyTypeId::Regular,EnemyTypeId::Fast,};   
            static const EnemyTypeId left_car2[]={EnemyTypeId::Toxic,EnemyTypeId::Heli,EnemyTypeId::Strong,EnemyTypeId::Fast,EnemyTypeId::Healer};
            static const EnemyTypeId left_car3[]={EnemyTypeId::Light,EnemyTypeId::Jet,EnemyTypeId::Fast,EnemyTypeId::Armored,EnemyTypeId::Strong};
            static const EnemyTypeId left_car4[]={EnemyTypeId::Icy,EnemyTypeId::Light,EnemyTypeId::Toxic,EnemyTypeId::Regular,EnemyTypeId::Fighter};
            static const EnemyTypeId right_car1[]={EnemyTypeId::Fighter,EnemyTypeId::Armored,EnemyTypeId::Toxic,EnemyTypeId::Healer,EnemyTypeId::Fast};
            static const EnemyTypeId right_car2[]={EnemyTypeId::Fighter,EnemyTypeId::Regular,EnemyTypeId::Fast,EnemyTypeId::Strong,EnemyTypeId::Heli};
            static const EnemyTypeId right_car3[]={EnemyTypeId::Light,EnemyTypeId::Regular,EnemyTypeId::Toxic,EnemyTypeId::Regular,EnemyTypeId::Fast};
            static const EnemyTypeId right_car4[]={EnemyTypeId::Regular,EnemyTypeId::Light,EnemyTypeId::Toxic,EnemyTypeId::Regular,EnemyTypeId::Fighter};
            for(int i=0;i<400;i++){
                SpawnGroup g11,g12,g13,g14,g21,g22,g23,g24;
                g11.count=1;
                g12.count=1;
                g13.count=1;
                g14.count=1;
                g21.count=1;
                g22.count=1;
                g23.count=1;
                g24.count=1;
                g11.startDelay=i*0.1f;
                g12.startDelay=i*0.1f;
                g13.startDelay=i*0.1f;
                g14.startDelay=i*0.1f;
                g21.startDelay=i*0.1f;
                g22.startDelay=i*0.1f;
                g23.startDelay=i*0.1f;
                g24.startDelay=i*0.1f;
                g11.interval=0.1f;
                g12.interval=0.1f;
                g13.interval=0.1f;
                g14.interval=0.1f;
                g21.interval=0.1f;
                g22.interval=0.1f;
                g23.interval=0.1f;
                g24.interval=0.1f;
                int lindex1=i%5;
                int lindex2=i%5;
                int lindex3=i%5;
                int lindex4=i%5;
                int rindex1=i%5;
                int rindex2=i%5;
                int rindex3=i%5;
                int rindex4=i%5;
                g11.type=left_car1[lindex1];
                g12.type=left_car2[lindex2];
                g13.type=left_car3[lindex3];
                g14.type=left_car4[lindex4];
                g21.type=right_car1[rindex1];
                g22.type=right_car2[rindex2];
                g23.type=right_car3[rindex3];
                g24.type=right_car4[rindex4];
                int spIdx1=0,spIdx2=0,spIdx3=0,spIdx4=0,spIdx5=0,spIdx6=0,spIdx7=0,spIdx8=0;
                int spidx=i%100;
                if(spidx<25){
                    spIdx1=25+spidx*2;
                    spIdx2=24-spidx;
                    spIdx3=74-spidx*2;
                    spIdx4=75+spidx;
                    spIdx5=spidx;
                    spIdx6=73-spidx*2;
                    spIdx7=99-spidx;
                    spIdx8=26+spidx*2;
                }else if(spidx<50){
                    spIdx1=75+(spidx-25);
                    spIdx2=74-(spidx-25)*2;
                    spIdx3=24-(spidx-25);
                    spIdx4=25+(spidx-25)*2;
                    spIdx5=26+(spidx-25)*2;
                    spIdx6=spidx-25;
                    spIdx7=73-(spidx-25)*2;
                    spIdx8=99-(spidx-25);
                }else if(spidx<75){
                    spIdx1=74-(spidx-50)*2;
                    spIdx2=24-(spidx-50);
                    spIdx3=25+(spidx-50)*2;
                    spIdx4=75+(spidx-50);
                    spIdx5=99-(spidx-50);
                    spIdx6=26+(spidx-50)*2;
                    spIdx7=spidx-50;
                    spIdx8=73-(spidx-50)*2;
                }else if(spidx<100){
                    spIdx1=24-(spidx-75);
                    spIdx2=25+(spidx-75)*2;
                    spIdx3=75+(spidx-75);
                    spIdx4=74-(spidx-75)*2;
                    spIdx5=73-(spidx-75)*2;
                    spIdx6=99-(spidx-75);
                    spIdx7=26+(spidx-75)*2;
                    spIdx8=spidx-75;
                }
                g11.spawnPointIndices={spIdx1};
                g12.spawnPointIndices={spIdx2};
                g13.spawnPointIndices={spIdx3};
                g14.spawnPointIndices={spIdx4};
                g21.spawnPointIndices={spIdx5};
                g22.spawnPointIndices={spIdx6};
                g23.spawnPointIndices={spIdx7};
                g24.spawnPointIndices={spIdx8};
                w6.groups.push_back(g11);
                w6.groups.push_back(g12);
                w6.groups.push_back(g13);
                w6.groups.push_back(g14);
                w6.groups.push_back(g21);
                w6.groups.push_back(g22);
                w6.groups.push_back(g23);
                w6.groups.push_back(g24);
            }
            lvl.waves.push_back(w6);
        }
        
        configs.push_back(lvl);
    }

    return configs;
}

const std::vector<LevelConfig> kLevelConfigs = buildLevelConfigs();

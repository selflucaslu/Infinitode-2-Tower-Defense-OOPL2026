#include "game/LevelConfig.hpp"
#include "enemy/EnemyTypeConfig.hpp"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <string>

// Helper to convert string back to EnemyTypeId
static EnemyTypeId parseEnemyTypeId(const std::string& name) {
    if (name == "Regular") return EnemyTypeId::Regular;
    if (name == "Fast") return EnemyTypeId::Fast;
    if (name == "Strong") return EnemyTypeId::Strong;
    if (name == "Heli") return EnemyTypeId::Heli;
    if (name == "Jet") return EnemyTypeId::Jet;
    if (name == "Armored") return EnemyTypeId::Armored;
    if (name == "Healer") return EnemyTypeId::Healer;
    if (name == "Toxic") return EnemyTypeId::Toxic;
    if (name == "Icy") return EnemyTypeId::Icy;
    if (name == "Fighter") return EnemyTypeId::Fighter;
    if (name == "Light") return EnemyTypeId::Light;
    throw std::runtime_error("Unknown EnemyTypeId: " + name);
}

// from_json overloads for nlohmann::json automatic deserialization
static void from_json(const nlohmann::json& j, SpawnGroup& g) {
    std::string typeStr = j.at("type").get<std::string>();
    g.type = parseEnemyTypeId(typeStr);
    g.count = j.at("count").get<int>();
    g.startDelay = j.at("startDelay").get<float>();
    g.interval = j.at("interval").get<float>();
    g.spawnPointIndices = j.at("spawnPointIndices").get<std::vector<int>>();
    if (j.contains("hpMultiplier")) {
        g.hpMultiplier = j.at("hpMultiplier").get<float>();
    } else {
        g.hpMultiplier = 1.0f;
    }
    if (j.contains("spdMultiplier")) {
        g.spdMultiplier = j.at("spdMultiplier").get<float>();
    } else {
        g.spdMultiplier = 1.0f;
    }
}

static void from_json(const nlohmann::json& j, WaveConfig& w) {
    w.waveNumber = j.at("waveNumber").get<int>();
    w.isBossWave = j.at("isBossWave").get<bool>();
    w.prepTime = j.at("prepTime").get<float>();
    w.clearRewardGold = j.at("clearRewardGold").get<int>();
    w.groups = j.at("groups").get<std::vector<SpawnGroup>>();
}

static void from_json(const nlohmann::json& j, LevelConfig& lvl) {
    lvl.levelNumber = j.at("levelNumber").get<int>();
    lvl.baseHp = j.at("baseHp").get<int>();
    lvl.startingGold = j.at("startingGold").get<int>();
    lvl.waves = j.at("waves").get<std::vector<WaveConfig>>();
}

// Dynamically read configs from assets/maps/*.json
static std::vector<LevelConfig> buildLevelConfigs() {
    std::vector<LevelConfig> configs;
    std::string mapsDir = "assets/maps";

    if (!std::filesystem::exists(mapsDir)) {
        return configs;
    }

    for (const auto& entry : std::filesystem::directory_iterator(mapsDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            std::string filename = entry.path().filename().string();
            if (filename.rfind("map_", 0) == 0) {
                std::string path = entry.path().generic_string();
                std::ifstream file(path);
                if (file.is_open()) {
                    try {
                        nlohmann::json j;
                        file >> j;
                        LevelConfig lvl = j.get<LevelConfig>();
                        lvl.mapPath = path;
                        configs.push_back(lvl);
                    } catch (const std::exception& e) {
                        throw std::runtime_error("Failed to parse map config " + path + ": " + e.what());
                    }
                }
            }
        }
    }

    // Sort level configurations by level number
    std::sort(configs.begin(), configs.end(), [](const LevelConfig& a, const LevelConfig& b) {
        return a.levelNumber < b.levelNumber;
    });

    return configs;
}

const std::vector<LevelConfig> kLevelConfigs = buildLevelConfigs();

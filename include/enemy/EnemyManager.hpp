#pragma once

#include "Util/Renderer.hpp"
#include "enemy/Enemy.hpp"
#include "map/GridMap.hpp"
#include "pathfinding/PythonPathfinder.hpp"
#include "utils/AtlasLoader.hpp"

#include <memory>
#include <utility>
#include <vector>

class EnemyManager {
public:
    EnemyManager(const GridMap& map, std::shared_ptr<AtlasLoader> atlasLoader);

    void update(float deltaTimeSec);
    void displayEnemies();
    void moveCamera(float dx, float dy);

private:
    void spawnEnemyFromLane(std::size_t laneIndex);
    std::vector<std::vector<int>> buildWalkabilityGrid() const;
    std::vector<std::pair<int, int>> collectSpawnTiles() const;
    std::vector<std::pair<int, int>> collectTargetTiles() const;

private:
    const GridMap& map;
    std::shared_ptr<AtlasLoader> atlasLoader;
    PythonPathfinder pathfinder;
    Util::Renderer enemyRoot;

    std::vector<std::vector<std::pair<float, float>>> laneWorldPaths;
    std::vector<int> laneSpawnedCount;

    std::vector<Enemy> enemies;
    std::vector<std::shared_ptr<Util::GameObject>> enemyObjects;
    std::vector<std::size_t> enemyLaneIndices;

    float cameraOffsetX = 0.0F;
    float cameraOffsetY = 0.0F;
    float spawnTimerSec = 0.0F;

    float spawnIntervalSec = 1.2F;
    int maxSpawnCountPerLane = 20;
};

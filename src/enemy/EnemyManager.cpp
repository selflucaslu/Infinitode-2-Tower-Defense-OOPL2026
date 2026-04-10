#include "enemy/EnemyManager.hpp"

#include "Util/Logger.hpp"

#include <cstddef>
#include <utility>
#include <vector>

EnemyManager::EnemyManager(const GridMap& map, std::shared_ptr<AtlasLoader> atlasLoader)
    : map(map), atlasLoader(std::move(atlasLoader)) {
    const std::vector<std::vector<int>> grid = buildWalkabilityGrid();
    const std::vector<std::pair<int, int>> starts = collectSpawnTiles();
    const std::vector<std::pair<int, int>> goals = collectTargetTiles();
    if (starts.empty()) {
        LOG_WARN("地圖沒有 spawn tile，怪物不會生成。");
    }
    if (goals.empty()) {
        LOG_WARN("地圖沒有 target tile，怪物不會生成。");
    }

    for (const std::pair<int, int>& start : starts) {
        const std::vector<std::pair<int, int>> pathTiles = pathfinder.findBestPath(grid, {start}, goals);
        if (pathTiles.empty()) {
            continue;
        }

        std::vector<std::pair<float, float>> worldPath;
        worldPath.reserve(pathTiles.size());
        for (const std::pair<int, int>& tile : pathTiles) {
            worldPath.emplace_back(this->map.getTileCenterWorld(tile.first, tile.second));
        }

        laneWorldPaths.emplace_back(std::move(worldPath));
        laneSpawnedCount.emplace_back(0);
    }

    if (starts.empty() || goals.empty() || laneWorldPaths.empty()) {
        LOG_WARN("Python 尋路沒有找到可用路徑，怪物不會生成。");
    } else {
        LOG_INFO("可用 spawn 路徑數量: {}", laneWorldPaths.size());
    }
}

void EnemyManager::update(float deltaTimeSec) {
    if (laneWorldPaths.empty()) {
        return;
    }

    spawnTimerSec += deltaTimeSec;
    while (spawnTimerSec >= spawnIntervalSec) {
        spawnTimerSec -= spawnIntervalSec;
        for (std::size_t laneIndex = 0; laneIndex < laneWorldPaths.size(); ++laneIndex) {
            if (laneSpawnedCount[laneIndex] >= maxSpawnCountPerLane) {
                continue;
            }
            spawnEnemyFromLane(laneIndex);
            laneSpawnedCount[laneIndex]++;
        }
    }

    std::size_t i = 0;
    while (i < enemies.size()) {
        Enemy& enemy = enemies[i];
        const std::size_t laneIndex = enemyLaneIndices[i];
        enemy.update(deltaTimeSec, laneWorldPaths[laneIndex]);

        enemyObjects[i]->m_Transform.translation = {
            enemy.getX() + cameraOffsetX,
            enemy.getY() + cameraOffsetY
        };

        if (!enemy.isAlive() || enemy.hasReachedGoal()) {
            auto objIt = enemyObjects.begin() + static_cast<std::ptrdiff_t>(i);
            enemyRoot.RemoveChild(*objIt);
            enemyObjects.erase(objIt);
            enemies.erase(enemies.begin() + static_cast<std::ptrdiff_t>(i));
            enemyLaneIndices.erase(enemyLaneIndices.begin() + static_cast<std::ptrdiff_t>(i));
            continue;
        }

        ++i;
    }
}

void EnemyManager::displayEnemies() {
    enemyRoot.Update();
}

void EnemyManager::moveCamera(float dx, float dy) {
    cameraOffsetX += dx;
    cameraOffsetY += dy;
}

void EnemyManager::spawnEnemyFromLane(std::size_t laneIndex) {
    if (laneIndex >= laneWorldPaths.size() || laneWorldPaths[laneIndex].empty()) {
        return;
    }

    const std::pair<float, float>& spawnPos = laneWorldPaths[laneIndex].front();
    enemies.emplace_back(
        spawnPos.first,
        spawnPos.second,
        70.0F,
        40,
        5,
        0,
        "enemy-type-regular"
    );

    std::shared_ptr<Util::GameObject> obj = std::make_shared<Util::GameObject>();
    obj->SetDrawable(atlasLoader->getImage("enemy-type-regular"));
    obj->SetZIndex(10.0F);
    obj->m_Transform.scale = {0.30F, 0.30F};
    obj->m_Transform.translation = {
        spawnPos.first + cameraOffsetX,
        spawnPos.second + cameraOffsetY
    };

    enemyObjects.emplace_back(obj);
    enemyLaneIndices.emplace_back(laneIndex);
    enemyRoot.AddChild(obj);
}

std::vector<std::vector<int>> EnemyManager::buildWalkabilityGrid() const {
    const int width = map.getMapWidth();
    const int height = map.getMapHeight();
    std::vector<std::vector<int>> grid(static_cast<size_t>(height), std::vector<int>(static_cast<size_t>(width), 1));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            grid[static_cast<size_t>(y)][static_cast<size_t>(x)] = map.getTile(x, y).getIsWalkable() ? 0 : 1;
        }
    }

    return grid;
}

std::vector<std::pair<int, int>> EnemyManager::collectSpawnTiles() const {
    const int width = map.getMapWidth();
    const int height = map.getMapHeight();

    std::vector<std::pair<int, int>> starts;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (map.getTile(x, y).isSpawnTile()) {
                starts.emplace_back(x, y);
            }
        }
    }
    return starts;
}

std::vector<std::pair<int, int>> EnemyManager::collectTargetTiles() const {
    const int width = map.getMapWidth();
    const int height = map.getMapHeight();

    std::vector<std::pair<int, int>> goals;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (map.getTile(x, y).isTargetTile()) {
                goals.emplace_back(x, y);
            }
        }
    }
    return goals;
}

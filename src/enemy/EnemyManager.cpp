#include "enemy/EnemyManager.hpp"

#include <algorithm>
#include <memory>
#include <optional>
#include <queue>
#include <stdexcept>
#include <vector>

// -------------------- 建立管理器 --------------------
EnemyManager::EnemyManager(const GridMap& map, AtlasLoader& atlasLoader)
    : m_Map(map), m_AtlasLoader(atlasLoader) {
    // 先建立固定路徑，確保後續生怪可直接綁定對應路徑。
    buildPathsFromMap();

    // 參照 GridMap 的計算方式，預先算出格子到世界座標的轉換基準。
    const std::shared_ptr<Util::Image> firstImage = m_AtlasLoader.getImage(m_Map.getTile(0, 0).getSpriteId());
    const glm::vec2 firstSize = firstImage->GetSize();
    m_CellW = firstSize.x * m_MapScale;
    m_CellH = firstSize.y * m_MapScale;
    m_StartX = -(m_Map.getMapWidth() * m_CellW) * 0.5F + m_CellW * 0.5F;
    m_StartY = -(m_Map.getMapHeight() * m_CellH) * 0.5F + m_CellH * 0.5F;
}

// -------------------- 生怪 --------------------
void EnemyManager::spawnEnemiesAt(
    const std::vector<std::size_t>& spawnPointIndices,
    float speed,
    int maxHealth,
    int damage,
    std::string_view spriteId
) {
    // 空陣列 = 所有起點都生怪（最直覺預設行為）。
    if (spawnPointIndices.empty()) {
        for (const std::shared_ptr<const std::vector<std::pair<int, int>>>& path : fixedPathPoints) {
            if (!path || path->empty()) {
                continue;
            }

            const float startX = path->front().first;
            const float startY = path->front().second;
            enemies.emplace_back(startX, startY, speed, maxHealth, damage, 0, std::string(spriteId), path);
        }
        return;
    }

    // 指定起點索引生怪（例如 {0}, {0,2}, {0,1,2}）。
    for (std::size_t spawnPointIndex : spawnPointIndices) {
        if (spawnPointIndex >= fixedPathPoints.size()) {
            continue;
        }

        const std::shared_ptr<const std::vector<std::pair<int, int>>>& path = fixedPathPoints[spawnPointIndex];
        if (!path || path->empty()) {
            continue;
        }

        const float startX = path->front().first;
        const float startY = path->front().second;
        enemies.emplace_back(startX, startY, speed, maxHealth, damage, 0, std::string(spriteId), path);
    }
}

// -------------------- 更新與渲染 --------------------
void EnemyManager::update(float deltaTime) {
    // 先更新敵人的邏輯位置（沿固定路徑移動）。
    for (Enemy& enemy : enemies) {
        enemy.update(deltaTime);
    }

    m_EnemyObjects.reserve(enemies.size());

    // 將渲染物件數量與敵人數量對齊。
    // 多出來的渲染物件要從 root 與容器移除。
    while (m_EnemyObjects.size() > enemies.size()) {
        m_EnemyRoot.RemoveChild(m_EnemyObjects.back());
        m_EnemyObjects.pop_back();
    }
    // 不足的渲染物件要補齊並掛到 root。
    while (m_EnemyObjects.size() < enemies.size()) {
        std::shared_ptr<Util::GameObject> enemyObject = std::make_shared<Util::GameObject>();
        enemyObject->SetZIndex(1.0F);
        m_EnemyRoot.AddChild(enemyObject);
        m_EnemyObjects.push_back(enemyObject);
    }

    // 同步每隻敵人的貼圖與世界座標（含 camera 偏移）。
    for (std::size_t enemyIndex = 0; enemyIndex < enemies.size(); enemyIndex++) {
        Enemy& enemy = enemies[enemyIndex];
        const std::shared_ptr<Util::GameObject>& enemyObject = m_EnemyObjects[enemyIndex];

        enemyObject->SetDrawable(m_AtlasLoader.getImage(enemy.getSpriteId()));
        enemyObject->m_Transform.scale = {m_MapScale, m_MapScale};
        enemyObject->m_Transform.translation = {
            m_StartX + enemy.getX() * m_CellW + m_CameraOffsetX,
            m_StartY + enemy.getY() * m_CellH + m_CameraOffsetY
        };
    }

    // 由 enemy root 統一提交繪製。
    m_EnemyRoot.Update();
}

// -------------------- 鏡頭移動 --------------------
void EnemyManager::moveCamera(float dx, float dy) {
    // 只更新相機偏移，不直接平移物件。
    // 物件最終位置統一在 update() 依 offset 重算，避免雙重位移。
    m_CameraOffsetX += dx;
    m_CameraOffsetY += dy;
}

// -------------------- 狀態收集與清理 --------------------
int EnemyManager::collectReachedGoalDamage() {
    // 只統計「這一幀已到終點」的敵人傷害值。
    // 外部通常會在扣血後呼叫 removeDeadAndReached() 清掉它們。
    int totalDamage = 0;
    for (const Enemy& enemy : enemies) {
        if (enemy.hasReachedGoal()) {
            totalDamage += enemy.getDamage();
        }
    }
    return totalDamage;
}

void EnemyManager::removeDeadEnemies() {
    // erase-remove_if：單次線性掃描清掉死亡敵人。
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& enemy) { return !enemy.isAlive(); }), enemies.end());
}

void EnemyManager::removeDeadAndReached() {
    // 一次清掉死亡或已到終點敵人，避免容器殘留無效個體。
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& enemy) { return !enemy.isAlive() || enemy.hasReachedGoal(); }), enemies.end());
}

// -------------------- 容器存取 --------------------
std::vector<Enemy>& EnemyManager::getEnemies() {
    return enemies;
}

const std::vector<Enemy>& EnemyManager::getEnemies() const {
    return enemies;
}

// -------------------- 固定路徑建立 --------------------
void EnemyManager::buildPathsFromMap() {
    fixedPathPoints.clear();
    const std::vector<std::pair<int, int>> spawnGridPoints = m_Map.getSpawnGridPoints();
    const std::optional<std::pair<int, int>> goalGridPointOpt = m_Map.getGoalGridPoint();
    const int mapWidth = m_Map.getMapWidth();
    const int mapHeight = m_Map.getMapHeight();

    if (spawnGridPoints.empty() || !goalGridPointOpt.has_value()) {
        throw std::runtime_error("地圖必須至少有一個起點和一個終點");
    }
    const std::pair<int, int> goalGridPoint = goalGridPointOpt.value();

    const std::vector<std::pair<int, int>> kFourDirs = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };

    // 對每個起點進行 BFS，找到可到終點的最短路徑。
    for (const std::pair<int, int>& spawn : spawnGridPoints) {
        std::queue<std::pair<std::pair<int, int>, std::vector<std::pair<int, int>>>> bfsQueue; // 元素是 (當前格座標, 從起點到當前格的路徑)
        std::vector<bool> visited(mapWidth * mapHeight, false); // 使用一維陣列來表示二維格子是否被訪問過

        std::vector<std::vector<std::pair<int, int>>> allGridPaths;
        bfsQueue.push({spawn, {spawn}});
        while (!bfsQueue.empty()) {
            const std::pair<std::pair<int, int>, std::vector<std::pair<int, int>>>& currentAndPath = bfsQueue.front();
            const std::pair<int, int>& current = currentAndPath.first;
            const std::vector<std::pair<int, int>>& path = currentAndPath.second;
            bfsQueue.pop();

            if (visited[current.second * mapWidth + current.first]) {
                continue;
            }

            if (current == goalGridPoint) {
                allGridPaths.push_back(path);
            }

            visited[current.second * mapWidth + current.first] = true; // 標記當前格子為已訪問
            for (const std::pair<int, int>& dir : kFourDirs) {
                const int dx = dir.first;
                const int dy = dir.second;
                std::pair<int, int> next{current.first + dx, current.second + dy};
                if (m_Map.canWalk(next.first, next.second) && !visited[next.second * mapWidth + next.first]) {
                    std::vector<std::pair<int, int>> newPath = path;
                    newPath.push_back(next);
                    bfsQueue.push({next, newPath});
                }
            }
        }

        // 從所有可行路徑中挑出最短的一條，並用 shared_ptr 保存。
        // 之後每隻敵人可共享這份路徑，避免重複複製。
        std::vector<std::pair<int, int>> shortestGridPath;
        for (const std::vector<std::pair<int, int>>& gridPath : allGridPaths) {
            if (shortestGridPath.empty() || gridPath.size() < shortestGridPath.size()) {
                shortestGridPath = gridPath;
            }
        }

        fixedPathPoints.push_back(std::make_shared<const std::vector<std::pair<int, int>>>(shortestGridPath));
    }
}

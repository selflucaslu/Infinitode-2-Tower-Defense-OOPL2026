#include "enemy/EnemyManager.hpp"

#include <memory>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// -------------------- 建立管理器 --------------------
EnemyManager::EnemyManager(const GridMap& map, AtlasLoader& atlasLoader)
    : m_Map(map), m_AtlasLoader(atlasLoader) {
    // 先建立固定路徑，確保後續生怪可直接綁定對應路徑。
    buildPathsFromMap();

    // 取第一個「非 Empty」格子當作基準尺寸，避免地圖左上角是 empty 時載圖失敗。
    std::string firstSpriteId;
    for (int y = 0; y < m_Map.getMapHeight(); ++y) {
        for (int x = 0; x < m_Map.getMapWidth(); ++x) {
            const Tile& tile = m_Map.getTile(x, y);
            if (tile.getType() != Tile::Type::Empty) {
                firstSpriteId = std::string(tile.getSpriteId());
                break;
            }
        }
        if (!firstSpriteId.empty()) {
            break;
        }
    }
    if (firstSpriteId.empty()) {
        throw std::runtime_error("地圖沒有可渲染格子，無法建立敵人座標基準");
    }

    const std::shared_ptr<Util::Image> firstImage = m_AtlasLoader.getImage(firstSpriteId);
    const glm::vec2 firstSize = firstImage->GetSize();
    m_CellW = firstSize.x * kMapScale;
    m_CellH = firstSize.y * kMapScale;
    m_StartX = -(m_Map.getMapWidth() * m_CellW) * 0.5F + m_CellW * 0.5F;
    m_StartY = -(m_Map.getMapHeight() * m_CellH) * 0.5F + m_CellH * 0.5F;
}

// -------------------- 生怪 --------------------
void EnemyManager::spawnEnemiesAt(
    const std::vector<int>& spawnPointIndices,
    float speed,
    Enemy::MoveType moveType,
    int maxHealth,
    int damage,
    int rewardGold,
    std::string_view spriteId
) {
    const int fixedPathCount = static_cast<int>(fixedPathPoints.size());

    // 空陣列 = 所有起點都生怪（最直覺預設行為）。
    if (spawnPointIndices.empty()) {
        for (const std::shared_ptr<const std::vector<std::pair<int, int>>>& path : fixedPathPoints) {
            if (!path || path->empty()) {
                continue;
            }

            const float startX = path->front().first;
            const float startY = path->front().second;
            enemies.emplace_back(startX, startY, speed, moveType, maxHealth, damage, rewardGold, 0, std::string(spriteId), path);
        }
        return;
    }

    // 指定起點索引生怪（例如 {0}, {0,2}, {0,1,2}）。
    for (int spawnPointIndex : spawnPointIndices) {
        if (spawnPointIndex < 0 || spawnPointIndex >= fixedPathCount) {
            continue;
        }

        const std::shared_ptr<const std::vector<std::pair<int, int>>>& path = fixedPathPoints[spawnPointIndex];
        if (!path || path->empty()) {
            continue;
        }

        const float startX = path->front().first;
        const float startY = path->front().second;
        enemies.emplace_back(startX, startY, speed, moveType, maxHealth, damage, rewardGold, 0, std::string(spriteId), path);
    }
}

// -------------------- 更新與渲染 --------------------
void EnemyManager::update(float deltaTime) {
    // 先更新敵人的邏輯位置（沿固定路徑移動）。
    for (Enemy& enemy : enemies) {
        enemy.update(deltaTime);
    }

    updateEnemyDisplay();
}

void EnemyManager::updateEnemyDisplay() {
    m_EnemyObjects.reserve(enemies.size());

    // 將渲染物件數量與敵人數量對齊。
    // 不足的渲染物件要補齊並掛到 root（固定屬性只在建立時設定一次）。
    while (m_EnemyObjects.size() < enemies.size()) {
        const int enemyIndex = static_cast<int>(m_EnemyObjects.size());
        Enemy& enemy = enemies[enemyIndex];
        std::shared_ptr<Util::GameObject> enemyObject = std::make_shared<Util::GameObject>();
        enemyObject->SetZIndex(
            enemy.getMoveType() == Enemy::MoveType::Air ? kAirEnemyZIndex : kGroundEnemyZIndex
        );
        enemyObject->SetDrawable(m_AtlasLoader.getImage(enemy.getSpriteId()));
        enemyObject->m_Transform.scale = {kMapScale, kMapScale};
        m_EnemyRoot.AddChild(enemyObject);
        m_EnemyObjects.push_back(enemyObject);
    }
    // 多出來的渲染物件要從 root 與容器移除。
    if (m_EnemyObjects.size() > enemies.size()) {
        while (m_EnemyObjects.size() > enemies.size()) {
            m_EnemyRoot.RemoveChild(m_EnemyObjects.back());
            m_EnemyObjects.pop_back();
        }
    }

    // 同步每隻敵人的貼圖與世界座標（含 camera 偏移）。
    const int enemyCount = static_cast<int>(enemies.size());
    for (int enemyIndex = 0; enemyIndex < enemyCount; enemyIndex++) {
        Enemy& enemy = enemies[enemyIndex];
        const std::shared_ptr<Util::GameObject>& enemyObject = m_EnemyObjects[enemyIndex];

        enemyObject->m_Transform.translation = {
            m_StartX + enemy.getX() * m_CellW + m_CameraOffsetX,
            m_StartY + enemy.getY() * m_CellH + m_CameraOffsetY
        };
    }

    const float cellW = m_CellW * m_CurrentScale;
    const float cellH = m_CellH * m_CurrentScale;

    // 動態計算當前縮放後的地圖左下角起點 (完美對齊 GridMap 的 updateTransforms 邏輯)
    const float startX = -(m_Map.getMapWidth() * cellW) * 0.5F + cellW * 0.5F + m_CameraOffsetX;
    const float startY = -(m_Map.getMapHeight() * cellH) * 0.5F + cellH * 0.5F + m_CameraOffsetY;

    // 同步每隻敵人的貼圖與世界座標
    const int enemyCount = static_cast<int>(enemies.size());
    for (int enemyIndex = 0; enemyIndex < enemyCount; enemyIndex++) {
        Enemy& enemy = enemies[enemyIndex];
        const std::shared_ptr<Util::GameObject>& enemyObject = m_EnemyObjects[enemyIndex];

        // 套用縮放比例 (原本只有 kMapScale，現在要乘上 m_CurrentScale)
        enemyObject->m_Transform.scale = {kMapScale * m_CurrentScale, kMapScale * m_CurrentScale};

        // 使用縮放後的起點與格子大小來計算真正的世界座標
        enemyObject->m_Transform.translation = {
            startX + enemy.getX() * cellW,
            startY + enemy.getY() * cellH
        };
    }

}

void EnemyManager::display() {
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
bool EnemyManager::isEnemysEmpty() {
    return enemies.empty();
}

EnemyManager::FrameResolveResult EnemyManager::collectFrameResolveResult() const {
    FrameResolveResult result;
    for (const Enemy& enemy : enemies) {
        // 到終點優先判定：同一隻不重複計成「到終點扣血 + 擊殺給錢」。
        if (enemy.hasReachedGoal()) {
            result.reachedGoalDamage += enemy.getDamage();
        } else if (!enemy.isAlive()) {
            result.killedRewardGold += enemy.getRewardGold();
        }
    }
    return result;
}


void EnemyManager::removeDeadAndReached() {
    // 由尾到頭同步刪除 enemies 與 m_EnemyObjects，避免剛清掉的敵人多顯示一幀。
    for (int enemyIndex = static_cast<int>(enemies.size()) - 1; enemyIndex >= 0; --enemyIndex) {
        if (!enemies[enemyIndex].isAlive() || enemies[enemyIndex].hasReachedGoal()) {
            enemies.erase(enemies.begin() + enemyIndex);
            if (enemyIndex < static_cast<int>(m_EnemyObjects.size())) {
                m_EnemyRoot.RemoveChild(m_EnemyObjects[enemyIndex]);
                m_EnemyObjects.erase(m_EnemyObjects.begin() + enemyIndex);
            }
        }
    }
}

EnemyManager::FrameResolveResult EnemyManager::resolveAndRemoveDeadAndReached() {
    FrameResolveResult result;

    // 線性壓縮：單次掃描同時完成
    // 1) 到終點傷害/擊殺金幣統計
    // 2) 移除死亡或已到終點敵人
    // 3) 同步維持 enemies / m_EnemyObjects 索引
    const int enemyCount = static_cast<int>(enemies.size());
    const int objectCount = static_cast<int>(m_EnemyObjects.size());
    std::vector<std::shared_ptr<Util::GameObject>> removedObjects;
    removedObjects.reserve(objectCount);
    int writeIndex = 0;
    for (int readIndex = 0; readIndex < enemyCount; ++readIndex) {
        Enemy& enemy = enemies[readIndex];
        if (enemy.hasReachedGoal()) {
            result.reachedGoalDamage += enemy.getDamage();
            if (readIndex < objectCount && m_EnemyObjects[readIndex]) {
                removedObjects.push_back(m_EnemyObjects[readIndex]);
            }
            continue;
        }
        if (!enemy.isAlive()) {
            result.killedRewardGold += enemy.getRewardGold();
            if (readIndex < objectCount && m_EnemyObjects[readIndex]) {
                removedObjects.push_back(m_EnemyObjects[readIndex]);
            }
            continue;
        }

        if (writeIndex != readIndex) {
            enemies[writeIndex] = std::move(enemies[readIndex]);
            if (readIndex < objectCount && writeIndex < objectCount) {
                m_EnemyObjects[writeIndex] = std::move(m_EnemyObjects[readIndex]);
            }
        }
        ++writeIndex;
    }

    if (writeIndex < enemyCount) {
        enemies.erase(enemies.begin() + writeIndex, enemies.end());
    }
    if (writeIndex < objectCount) {
        m_EnemyObjects.erase(m_EnemyObjects.begin() + writeIndex, m_EnemyObjects.end());
    }
    for (const std::shared_ptr<Util::GameObject>& removedObject : removedObjects) {
        m_EnemyRoot.RemoveChild(removedObject);
    }

    return result;
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
        std::vector<std::pair<int, int>> shortestGridPath;

        bfsQueue.push({spawn, {spawn}});
        visited[spawn.second * mapWidth + spawn.first] = true;
        while (!bfsQueue.empty()) {
            const std::pair<std::pair<int, int>, std::vector<std::pair<int, int>>> currentAndPath = bfsQueue.front();
            const std::pair<int, int>& current = currentAndPath.first;
            const std::vector<std::pair<int, int>>& path = currentAndPath.second;
            bfsQueue.pop();

            if (current == goalGridPoint) {
                // BFS 第一次到達終點即為最短路徑，可直接結束。
                shortestGridPath = path;
                break;
            }

            for (const std::pair<int, int>& dir : kFourDirs) {
                const int dx = dir.first;
                const int dy = dir.second;
                std::pair<int, int> next{current.first + dx, current.second + dy};
                if (m_Map.canWalk(next.first, next.second) && !visited[next.second * mapWidth + next.first]) {
                    visited[next.second * mapWidth + next.first] = true; // enqueue 時就標記，避免重複入隊
                    std::vector<std::pair<int, int>> newPath = path;
                    newPath.push_back(next);
                    bfsQueue.push({next, newPath});
                }
            }
        }

        if (shortestGridPath.empty()) {
            throw std::runtime_error(
                "路徑不可達: spawn=(" + std::to_string(spawn.first) + "," + std::to_string(spawn.second) +
                "), goal=(" + std::to_string(goalGridPoint.first) + "," + std::to_string(goalGridPoint.second) + ")"
            );
        }

        fixedPathPoints.push_back(std::make_shared<const std::vector<std::pair<int, int>>>(shortestGridPath));
    }
}

// 1. 新增 zoomCamera 實作
void EnemyManager::zoomCamera(float zoomDelta) {
    float zoomFactor = (zoomDelta > 0.0F) ? 1.05F : 0.95F;
    m_CurrentScale *= zoomFactor;

    // 保持與 GridMap 相同的縮放限制
    if (m_CurrentScale < 0.1F) m_CurrentScale = 0.1F;
    if (m_CurrentScale > 3.0F) m_CurrentScale = 3.0F;
}
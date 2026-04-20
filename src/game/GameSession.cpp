#include "game/GameSession.hpp"
#include "game/LevelConfig.hpp"
#include "Util/Logger.hpp"

#include <array>
#include <cstddef>

// -------------------- 建立單局 --------------------
GameSession::GameSession(int levelNumber) {
    // 先載入圖集，再建立地圖。
    atlasLoader = std::make_unique<AtlasLoader>();
    atlasLoader->loadAtlas("assets/combined.atlas");

    // 讀取單局資料
    const LevelConfig& level = getLevelConfig(levelNumber);
  
    // 建立地圖與敵人管理器，並傳入 atlasLoader 參考（共用資源）。
    map = std::make_unique<GridMap>(level.mapPath, *atlasLoader);
    enemyManager = std::make_unique<EnemyManager>(*map, *atlasLoader);
    towerManager = std::make_unique<TowerManager>(*map);

    // 預熱常用敵人貼圖，避免第一次出場時發生卡頓。
    const std::array<EnemyTypeId, 5> preloadEnemyTypes = {
        EnemyTypeId::Regular,
        EnemyTypeId::Fast,
        EnemyTypeId::Strong,
        EnemyTypeId::Heli,
        EnemyTypeId::Jet
    };
    for (EnemyTypeId enemyTypeId : preloadEnemyTypes) {
        const EnemyTypeConfig& config = getEnemyTypeConfig(enemyTypeId);
        (void)atlasLoader->getImage(config.spriteId);
    }

    // 初始化遊戲狀態（基地血量與金幣初始值從配置讀取）。
    initBaseHp = level.baseHp;
    initGold = level.startingGold;
    spawnSchedule = level.waves;
    initSession();

    // 背景改為 Infinitode 風格的灰色同色系 #181818。
    glClearColor(24.0F / 255.0F, 24.0F / 255.0F, 24.0F / 255.0F, 1.0F);
}

// -------------------- 地圖存取 --------------------
GridMap& GameSession::getMap() {
    return *map;
}

const GridMap& GameSession::getMap() const {
    return *map;
}

// -------------------- 敵人管理器存取 --------------------
EnemyManager& GameSession::getEnemyManager() {
    return *enemyManager;
}

const EnemyManager& GameSession::getEnemyManager() const {
    return *enemyManager;
}

// -------------------- 塔管理器存取 --------------------
TowerManager& GameSession::getTowerManager() {
    return *towerManager;
}

const TowerManager& GameSession::getTowerManager() const {
    return *towerManager;
}

bool GameSession::placeTower(int gridX, int gridY, std::string_view spriteId) {
    if (!towerManager->placeTower(gridX, gridY, spriteId)) {
        return false;
    }

    updateTowerDisplay();
    return true;
}

// -------------------- 基地血量 --------------------
int GameSession::getBaseHp() const {
    return baseHp;
}

int GameSession::getGold() const {
    return gold;
}

void GameSession::setBaseHp(int newBaseHp) {
    baseHp = newBaseHp;
}

void GameSession::applyBaseDamage(int damage) {
    if (damage <= 0) {
        return;
    }

    baseHp -= damage;
    if (baseHp < 0) {
        baseHp = 0;
    }
}

void GameSession::addGold(int amount) {
    if (amount <= 0) {
        return;
    }
    gold += amount;
}

bool GameSession::isBaseAlive() const {
    return baseHp > 0;
}

// -------------------- 波次 --------------------
int GameSession::getWave() const {
    return waveCount;
}

void GameSession::setWave(int newWave) {
    waveCount = newWave;
}

void GameSession::nextWave() {
    waveCount += 1;
}

// -------------------- 每幀流程 --------------------
void GameSession::update(float deltaTime) {
    if (!isSessionActive) {
        // 暫停時仍要同步敵人渲染座標，讓相機移動時敵人能跟著地圖一起平移。
        enemyManager->updateEnemyDisplay();
        enemyManager->updateTowerDisplay();
        return; // 如果遊戲未啟動，跳過更新。
    }
    timer += deltaTime;
    waveTimer += deltaTime;
    groupTimer += deltaTime;
    dispatchEnemiesByTimer();

    // EnemyManager 處理敵人更新與渲染提交。
    enemyManager->update(deltaTime);
    towerManager->updateAutoAttack(deltaTime, enemyManager->getEnemies());

    // 每幀單次掃描完成「收集結果 + 清理」，避免重複遍歷 enemies。
    const EnemyManager::FrameResolveResult frameResult = enemyManager->resolveAndRemoveDeadAndReached();
    applyBaseDamage(frameResult.reachedGoalDamage);
    addGold(frameResult.killedRewardGold);
    if (!isBaseAlive()) {
        pauseSession();
        LOG_INFO("[Session] Game Over");
        return; // 阻止清波獎勵與進下一波
    }

    // 本波清空判定必須放在清理後，避免最後一隻剛消失時慢一幀才切下一波。
    if (waveCount >= 0 && waveCount < static_cast<int>(spawnSchedule.size())) {
        const WaveConfig& waveConfig = spawnSchedule[waveCount];
        const int groupCount = static_cast<int>(waveConfig.groups.size());
        const bool isWaveSpawnFinished = groupIndex >= groupCount;
        if (isWaveSpawnFinished && enemyManager->getEnemies().empty()) {
            addGold(waveConfig.clearRewardGold);
            nextWave();
            waveTimer = 0.0F;
            groupTimer = 0.0F;
            groupIndex = 0;
            groupSpawned = 0;
        }
    }
}

void GameSession::display() {
    // 先畫地圖，再畫塔與敵人。
    map->displayMap();
    updateTowerDisplay();
    towerRoot.Update();
    enemyManager->display();
}

void GameSession::moveCamera(float dx, float dy) {
    // 兩者雖然實作方式不同，但只要同一幀用相同 dx/dy 呼叫，
    // 地圖（直接位移）與敵人（offset 重算）會保持同步。
    map->moveCamera(dx, dy);
    enemyManager->moveCamera(dx, dy);
}

// -------------------- 遊戲流程控制 --------------------
void GameSession::initSession() {
    isSessionActive = false;
    timer = 0.0F;
    waveTimer = 0.0F;
    groupTimer = 0.0F;
    baseHp = initBaseHp;
    gold = initGold;
    waveCount = 0;
    groupIndex = 0;
    groupSpawned = 0;
    enemyManager->getEnemies().clear();
    towerManager->getTowers().clear();
    updateTowerDisplay();
    LOG_INFO("[Session] init: baseHp={}, gold={}", baseHp, gold);
}

void GameSession::startSession() {
    isSessionActive = true;
}

void GameSession::pauseSession() {
    isSessionActive = false;
}

void GameSession::dispatchEnemiesByTimer() {
    // 1) 先確認波次合法。
    if (waveCount < 0 || waveCount >= static_cast<int>(spawnSchedule.size())) {
        return;
    }

    const WaveConfig& waveConfig = spawnSchedule[waveCount];
    const int groupCount = static_cast<int>(waveConfig.groups.size());

    // 2) 先等本波 prepTime。
    if (waveTimer >= waveConfig.prepTime) {
        // 3) 只處理目前 groupIndex 這一組（依序跑，不並行）。
        if (groupIndex < groupCount) {
            const SpawnGroup& spawnGroup = waveConfig.groups[groupIndex];
            const EnemyTypeConfig& config = getEnemyTypeConfig(spawnGroup.type);

            // 4) 第一隻：等到 prepTime + startDelay。
            if (groupSpawned == 0) {
                if (waveTimer >= waveConfig.prepTime + spawnGroup.startDelay) {
                    enemyManager->spawnEnemiesAt(
                        spawnGroup.spawnPointIndices,
                        config.speed,
                        config.moveType,
                        config.maxHealth,
                        config.damageToBase,
                        config.rewardGold,
                        config.spriteId
                    );
                    groupSpawned += 1;
                    groupTimer = 0.0F;
                }
            } else {
                // 5) 後續：用 groupTimer + interval 控制每次再生一隻。
                if (spawnGroup.interval <= 0.0F || groupTimer >= spawnGroup.interval) {
                    enemyManager->spawnEnemiesAt(
                        spawnGroup.spawnPointIndices,
                        config.speed,
                        config.moveType,
                        config.maxHealth,
                        config.damageToBase,
                        config.rewardGold,
                        config.spriteId
                    );
                    groupSpawned += 1;
                    groupTimer = 0.0F;
                }
            }

            // 6) 這組完成就切下一組。
            if (groupSpawned >= spawnGroup.count) {
                groupIndex += 1;
                groupSpawned = 0;
                groupTimer = 0.0F;
            }
        }
    }

}

void GameSession::updateTowerDisplay() {
    const std::vector<Tower>& towers = towerManager->getTowers();
    towerObjects.reserve(towers.size());

    while (towerObjects.size() < towers.size()) {
        std::shared_ptr<Util::GameObject> towerObject = std::make_shared<Util::GameObject>();
        towerObject->SetZIndex(kTowerZIndex);
        towerObject->m_Transform.scale = {kTowerScale, kTowerScale};
        towerRoot.AddChild(towerObject);
        towerObjects.push_back(towerObject);
    }

    while (towerObjects.size() > towers.size()) {
        towerRoot.RemoveChild(towerObjects.back());
        towerObjects.pop_back();
    }

    for (std::size_t i = 0; i < towers.size(); ++i) {
        const Tower& tower = towers[i];
        const std::shared_ptr<Util::GameObject>& towerObject = towerObjects[i];

        towerObject->SetDrawable(atlasLoader->getImage(tower.GetspriteId()));
        const std::optional<glm::vec2> worldPos = map->gridToWorld(tower.GetGridX(), tower.GetGridY());
        if (!worldPos.has_value()) {
            continue;
        }
        towerObject->m_Transform.translation = worldPos.value();
    }
}

// -------------------- 測試入口 --------------------
void GameSession::spawnDebugEnemy(
    EnemyTypeId enemyTypeId,
    const std::vector<int>& spawnPointIndices
) {
    const EnemyTypeConfig& config = getEnemyTypeConfig(enemyTypeId);
    enemyManager->spawnEnemiesAt(
        spawnPointIndices,
        config.speed,
        config.moveType,
        config.maxHealth,
        config.damageToBase,
        config.rewardGold,
        config.spriteId
    );
}

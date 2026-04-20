#include "game/GameSession.hpp"
#include "game/LevelConfig.hpp"
#include "Core/Context.hpp"
#include "Util/Color.hpp"
#include "Util/Logger.hpp"

#include <array>
#include <cstddef>
#include <string>

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

    // 預熱目前會用到的塔與子彈貼圖，避免第一次建塔/開火時卡頓。
    const std::array<std::string_view, 4> preloadTowerSprites = {
        "tower-basic",
        "tower-basic-base",
        "tower-basic-weapon",
        "projectile-basic"
    };
    for (std::string_view spriteId : preloadTowerSprites) {
        (void)atlasLoader->getImage(spriteId);
    }

    // 初始化遊戲狀態（基地血量與金幣初始值從配置讀取）。
    initBaseHp = level.baseHp;
    initGold = level.startingGold;
    spawnSchedule = level.waves;
    initSession();

    towerHpText = std::make_shared<Util::Text>(
        kHudFontPath, kHudFontSize, "基地生命: 0", Util::Color::FromRGB(255, 255, 255)
    );
    towerHpTextObject = std::make_shared<Util::GameObject>();
    towerHpTextObject->SetDrawable(towerHpText);
    towerHpTextObject->SetZIndex(kHudZIndex);

    goldText = std::make_shared<Util::Text>(
        kHudFontPath, kHudFontSize, "金幣: 0", Util::Color::FromRGB(255, 255, 255)
    );
    goldTextObject = std::make_shared<Util::GameObject>();
    goldTextObject->SetDrawable(goldText);
    goldTextObject->SetZIndex(kHudZIndex);

    towerHpIconObject = std::make_shared<Util::GameObject>();
    towerHpIconObject->SetDrawable(atlasLoader->getImage("icon-heart"));
    towerHpIconObject->m_Transform.scale = {kHudIconScale, kHudIconScale};
    towerHpIconObject->SetZIndex(kHudZIndex);

    goldIconObject = std::make_shared<Util::GameObject>();
    goldIconObject->SetDrawable(atlasLoader->getImage("icon-coins"));
    goldIconObject->m_Transform.scale = {kHudIconScale, kHudIconScale};
    goldIconObject->SetZIndex(kHudZIndex);

    hudRoot.AddChild(towerHpIconObject);
    hudRoot.AddChild(towerHpTextObject);
    hudRoot.AddChild(goldIconObject);
    hudRoot.AddChild(goldTextObject);
    updateHudDisplay();

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
    if (gold < kTowerBuildCost) {
        return false;
    }

    if (!towerManager->placeTower(gridX, gridY, spriteId)) {
        return false;
    }

    gold -= kTowerBuildCost;
    updateTowerDisplay();
    updateHudDisplay();
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
        updateTowerDisplay();
        updateProjectileDisplay();
        updateHudDisplay();
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
    updateProjectileDisplay();
    projectileRoot.Update();
    enemyManager->display();
    updateHudDisplay();
    hudRoot.Update();
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
    towerManager->clear();
    updateTowerDisplay();
    updateProjectileDisplay();
    updateHudDisplay();
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
    towerBaseObjects.reserve(towers.size());
    towerWeaponObjects.reserve(towers.size());

    while (towerBaseObjects.size() < towers.size()) {
        std::shared_ptr<Util::GameObject> towerBaseObject = std::make_shared<Util::GameObject>();
        towerBaseObject->SetZIndex(kTowerBaseZIndex);
        towerBaseObject->m_Transform.scale = {kTowerScale, kTowerScale};
        towerRoot.AddChild(towerBaseObject);
        towerBaseObjects.push_back(towerBaseObject);

        std::shared_ptr<Util::GameObject> towerWeaponObject = std::make_shared<Util::GameObject>();
        towerWeaponObject->SetZIndex(kTowerWeaponZIndex);
        towerWeaponObject->m_Transform.scale = {kTowerScale, kTowerScale};
        towerRoot.AddChild(towerWeaponObject);
        towerWeaponObjects.push_back(towerWeaponObject);
    }

    while (towerBaseObjects.size() > towers.size()) {
        towerRoot.RemoveChild(towerBaseObjects.back());
        towerBaseObjects.pop_back();

        towerRoot.RemoveChild(towerWeaponObjects.back());
        towerWeaponObjects.pop_back();
    }

    for (std::size_t i = 0; i < towers.size(); ++i) {
        const Tower& tower = towers[i];
        const std::shared_ptr<Util::GameObject>& towerBaseObject = towerBaseObjects[i];
        const std::shared_ptr<Util::GameObject>& towerWeaponObject = towerWeaponObjects[i];

        towerBaseObject->SetDrawable(atlasLoader->getImage("tower-basic-base"));
        towerWeaponObject->SetDrawable(atlasLoader->getImage("tower-basic-weapon"));
        const std::optional<glm::vec2> worldPos = map->gridToWorld(tower.GetGridX(), tower.GetGridY());
        if (!worldPos.has_value()) {
            continue;
        }
        towerBaseObject->m_Transform.translation = worldPos.value();
        towerBaseObject->m_Transform.rotation = 0.0F;

        towerWeaponObject->m_Transform.translation = worldPos.value();
        towerWeaponObject->m_Transform.rotation = tower.GetFacingRotation();
    }
}

void GameSession::updateProjectileDisplay() {
    const std::vector<TowerManager::Projectile>& projectiles = towerManager->getProjectiles();
    projectileObjects.reserve(projectiles.size());

    while (projectileObjects.size() < projectiles.size()) {
        std::shared_ptr<Util::GameObject> projectileObject = std::make_shared<Util::GameObject>();
        projectileObject->SetZIndex(kProjectileZIndex);
        projectileObject->m_Transform.scale = {kProjectileScale, kProjectileScale};
        projectileObject->SetDrawable(atlasLoader->getImage("projectile-basic"));
        projectileRoot.AddChild(projectileObject);
        projectileObjects.push_back(projectileObject);
    }

    while (projectileObjects.size() > projectiles.size()) {
        projectileRoot.RemoveChild(projectileObjects.back());
        projectileObjects.pop_back();
    }

    for (std::size_t i = 0; i < projectiles.size(); ++i) {
        const TowerManager::Projectile& projectile = projectiles[i];
        const std::shared_ptr<Util::GameObject>& projectileObject = projectileObjects[i];
        const std::optional<glm::vec2> worldPos = map->gridToWorld(projectile.x, projectile.y);
        if (!worldPos.has_value()) {
            continue;
        }
        projectileObject->m_Transform.translation = worldPos.value();
    }
}

void GameSession::updateHudDisplay() {
    if (!towerHpText || !goldText || !towerHpIconObject || !goldIconObject || !towerHpTextObject || !goldTextObject) {
        return;
    }

    towerHpText->SetText("基地生命: " + std::to_string(baseHp));
    goldText->SetText("金幣: " + std::to_string(gold));

    const std::shared_ptr<Core::Context> context = Core::Context::GetInstance();
    const float halfWindowWidth = static_cast<float>(context->GetWindowWidth()) * 0.5F;
    const float halfWindowHeight = static_cast<float>(context->GetWindowHeight()) * 0.5F;

    const glm::vec2 towerIconSize = atlasLoader->getImage("icon-heart")->GetSize();
    const glm::vec2 goldIconSize = atlasLoader->getImage("icon-coins")->GetSize();
    const glm::vec2 towerTextSize = towerHpText->GetSize();
    const glm::vec2 goldTextSize = goldText->GetSize();

    const float topY = halfWindowHeight - kHudPadding;

    towerHpIconObject->m_Transform.translation = {
        -halfWindowWidth + kHudPadding + towerIconSize.x * kHudIconScale * 0.5F,
        topY - towerIconSize.y * kHudIconScale * 0.5F
    };
    towerHpTextObject->m_Transform.translation = {
        towerHpIconObject->m_Transform.translation.x + towerIconSize.x * kHudIconScale * 0.5F + kHudGap + towerTextSize.x * 0.5F,
        topY - towerTextSize.y * 0.5F
    };

    goldTextObject->m_Transform.translation = {
        halfWindowWidth - kHudPadding - goldTextSize.x * 0.5F,
        topY - goldTextSize.y * 0.5F
    };
    goldIconObject->m_Transform.translation = {
        goldTextObject->m_Transform.translation.x - goldTextSize.x * 0.5F - kHudGap - goldIconSize.x * kHudIconScale * 0.5F,
        topY - goldIconSize.y * kHudIconScale * 0.5F
    };
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

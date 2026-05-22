#include "game/GameSession.hpp"
#include "game/LevelConfig.hpp"
#include "Core/Context.hpp"
#include "Util/Color.hpp"
#include "Util/Logger.hpp"

#include <array>
#include <cstddef>
#include <string>

// -------------------- 建立單局 --------------------
GameSession::GameSession(int levelNumber)
    : isSessionActive(false), timer(0.0F), waveTimer(0.0F), groupTimer(0.0F),
      baseHp(0), gold(0), waveCount(0), groupIndex(0), groupSpawned(0),
      loopCount(0) {

    // 先載入圖集，再建立地圖。
    atlasLoader = std::make_unique<AtlasLoader>();
    atlasLoader->loadAtlas("assets/combined.atlas");

    // 讀取單局資料
    const LevelConfig& level = getLevelConfig(levelNumber);

    // 建立地圖與敵人管理器，並傳入 atlasLoader 參考（共用資源）。
    map = std::make_unique<GridMap>(level.mapPath, *atlasLoader);
    enemyManager = std::make_unique<EnemyManager>(*map, *atlasLoader);
    towerManager = std::make_unique<TowerManager>(*map);

    static constexpr std::array<EnemyTypeId, 5> preloadEnemyTypes = {
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

    static constexpr std::array<std::string_view, 10> preloadTowerSprites = {
        "tower-basic-base",
        "tower-basic-weapon",
        "tower-sniper-base-new",
        "tower-sniper-weapon-new",
        "tower-cannon-base-new",
        "tower-cannon-weapon-new",
        "projectile-basic",
        "icon-tower",
        "icon-tower-top",
        "aura-range"
    };
    for (std::string_view spriteId : preloadTowerSprites) {
        (void)atlasLoader->getImage(spriteId);
    }

    // 初始化遊戲狀態
    initBaseHp = level.baseHp;
    initGold = level.startingGold;
    baseSpawnSchedule = level.waves; // 保存原始配置，供後續循環計算用
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

    waveText = std::make_shared<Util::Text>(
        kHudFontPath, kHudFontSize, "波次: 0", Util::Color::FromRGB(255, 255, 255)
    );
    waveTextObject = std::make_shared<Util::GameObject>();
    waveTextObject->SetDrawable(waveText);
    waveTextObject->SetZIndex(kHudZIndex);

    waveIconObject = std::make_shared<Util::GameObject>();
    waveIconObject->SetDrawable(atlasLoader->getImage("icon-flag"));
    waveIconObject->m_Transform.scale = {kHudIconScale, kHudIconScale};
    waveIconObject->SetZIndex(kHudZIndex);

    hudRoot.AddChild(towerHpIconObject);
    hudRoot.AddChild(towerHpTextObject);
    hudRoot.AddChild(goldIconObject);
    hudRoot.AddChild(goldTextObject);
    hudRoot.AddChild(waveIconObject);
    hudRoot.AddChild(waveTextObject);
    updateHudDisplay();

    // 背景改為 Infinitode 風格的灰色同色系 #181818。
    glClearColor(24.0F / 255.0F, 24.0F / 255.0F, 24.0F / 255.0F, 1.0F);

    // 建立選塔面板（必須在 atlasLoader 內容完成後）
    m_SelectionPanel = std::make_unique<TowerSelectionPanel>(*atlasLoader);
}

// -------------------- 地圖存取 --------------------
GridMap& GameSession::getMap() { return *map; }
const GridMap& GameSession::getMap() const { return *map; }

// -------------------- 敵人管理器存取 --------------------
EnemyManager& GameSession::getEnemyManager() { return *enemyManager; }
const EnemyManager& GameSession::getEnemyManager() const { return *enemyManager; }

// -------------------- 塔管理器存取 --------------------
TowerManager& GameSession::getTowerManager() { return *towerManager; }
const TowerManager& GameSession::getTowerManager() const { return *towerManager; }

bool GameSession::placeTower(int gridX, int gridY, TowerId towerId) {
    const TowerDef& def = getTowerDef(towerId);
    if (gold < def.buildCost) { return false; }
    if (!towerManager->placeTower(gridX, gridY, towerId)) { return false; }
    gold -= def.buildCost;
    updateTowerDisplay();
    updateHudDisplay();
    return true;
}

bool GameSession::sellTower(int gridX, int gridY) {
    auto towerIdOpt = towerManager->getTowerIdAt(gridX, gridY);
    if (!towerIdOpt.has_value()) return false;

    const TowerDef& def = getTowerDef(towerIdOpt.value());
    int refund = def.buildCost / 2;

    if (towerManager->removeTower(gridX, gridY)) {
        addGold(refund);
        updateTowerDisplay();
        updateHudDisplay();
        return true;
    }
    return false;
}

// -------------------- 選塔面板 --------------------
void GameSession::setSelectedTower(TowerId id) {
    if (m_SelectionPanel) { m_SelectionPanel->setSelectedTower(id); }
}

TowerId GameSession::getSelectedTower() const {
    if (m_SelectionPanel) { return m_SelectionPanel->getSelectedTower(); }
    return TowerId::Basic;
}

std::optional<TowerId> GameSession::hitTestSelectionPanel(float screenX, float screenY) const {
    if (!m_SelectionPanel) return std::nullopt;
    return m_SelectionPanel->hitTest(screenX, screenY);
}

// -------------------- 基地血量 --------------------
int GameSession::getBaseHp() const { return baseHp; }
int GameSession::getGold() const { return gold; }
void GameSession::setBaseHp(int newBaseHp) { baseHp = newBaseHp; }

void GameSession::applyBaseDamage(int damage) {
    if (damage <= 0) return;
    baseHp -= damage;
    if (baseHp < 0) baseHp = 0;
}

void GameSession::addGold(int amount) {
    if (amount <= 0) return;
    gold += amount;
}

bool GameSession::isBaseAlive() const { return baseHp > 0; }

// -------------------- 波次 --------------------
int GameSession::getWave() const { return waveCount; }
void GameSession::setWave(int newWave) { waveCount = newWave; }
void GameSession::nextWave() { waveCount += 1; }

// -------------------- 每幀流程 --------------------
void GameSession::update(float deltaTime) {
    if (!isSessionActive) {
        enemyManager->updateEnemyDisplay(map->getOffsetX(), map->getOffsetY(), map->getCurrentScale());
        updateTowerDisplay();
        updateProjectileDisplay();
        updateHudDisplay();
        return;
    }
    timer += deltaTime;
    waveTimer += deltaTime;
    groupTimer += deltaTime;
    dispatchEnemiesByTimer();

    enemyManager->update(deltaTime);
    towerManager->updateAutoAttack(deltaTime, enemyManager->getEnemies());
    enemyManager->updateEnemyDisplay(map->getOffsetX(), map->getOffsetY(), map->getCurrentScale());

    const EnemyManager::FrameResolveResult frameResult = enemyManager->resolveAndRemoveDeadAndReached();
    applyBaseDamage(frameResult.reachedGoalDamage);
    addGold(frameResult.killedRewardGold);
    if (!isBaseAlive()) {
        pauseSession();
        LOG_INFO("[Session] Game Over");
        return;
    }

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
    } else if (waveCount >= static_cast<int>(spawnSchedule.size()) &&
               enemyManager->getEnemies().empty()) {
        // 所有波次完成且場上無敵人 → 開始下一輪循環
        beginNextLoop();
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void GameSession::display() {
    map->displayMap();
    updateTowerDisplay();
    towerRoot.Update();
    updateProjectileDisplay();
    projectileRoot.Update();
    enemyManager->display();
    updateHudDisplay();
    hudRoot.Update();
    // 選塔面板（最後繪製，不被地圖或敏人遮擋）
    if (m_SelectionPanel) { m_SelectionPanel->display(); }
}

void GameSession::moveCamera(float dx, float dy) const {
    map->moveCamera(dx, dy);
    // ★ 已移除錯誤呼叫：enemyManager->moveCamera(dx, dy);
}

void GameSession::zoomCamera(float zoomDelta) const {
    map->zoomCamera(zoomDelta);
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
    loopCount = 0;
    spawnSchedule = baseSpawnSchedule; // 重置回第一輪原始配置
    enemyManager->getEnemies().clear();
    towerManager->clear();
    updateTowerDisplay();
    updateProjectileDisplay();
    updateHudDisplay();
    LOG_INFO("[Session] init: baseHp={}, gold={}", baseHp, gold);
}

void GameSession::startSession() { isSessionActive = true; }
void GameSession::pauseSession() { isSessionActive = false; }

void GameSession::dispatchEnemiesByTimer() {
    if (waveCount < 0 || waveCount >= static_cast<int>(spawnSchedule.size())) return;

    const WaveConfig& waveConfig = spawnSchedule[waveCount];
    const int groupCount = static_cast<int>(waveConfig.groups.size());

    if (waveTimer >= waveConfig.prepTime) {
        if (groupIndex < groupCount) {
            const SpawnGroup& spawnGroup = waveConfig.groups[groupIndex];
            const EnemyTypeConfig& config = getEnemyTypeConfig(spawnGroup.type);

            // 套用循環強化倍率
            const float scaledSpeed  = config.speed     * spawnGroup.spdMultiplier;
            const int   scaledHp     = static_cast<int>(static_cast<float>(config.maxHealth) * spawnGroup.hpMultiplier);

            if (groupSpawned == 0) {
                if (waveTimer >= waveConfig.prepTime + spawnGroup.startDelay) {
                    enemyManager->spawnEnemiesAt(spawnGroup.spawnPointIndices, scaledSpeed, config.moveType, scaledHp, config.damageToBase, config.rewardGold, config.spriteId);
                    groupSpawned += 1;
                    groupTimer = 0.0F;
                }
            } else {
                if (spawnGroup.interval <= 0.0F || groupTimer >= spawnGroup.interval) {
                    enemyManager->spawnEnemiesAt(spawnGroup.spawnPointIndices, scaledSpeed, config.moveType, scaledHp, config.damageToBase, config.rewardGold, config.spriteId);
                    groupSpawned += 1;
                    groupTimer = 0.0F;
                }
            }

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
    const float currentScale = map->getCurrentScale();

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

        towerBaseObject->SetDrawable(atlasLoader->getImage(tower.GetspriteId()));
        // 尋找對應武器：將 base spriteId 轉招 weapon spriteId
        std::string weaponId = tower.GetspriteId();
        // 替換 "base" 為 "weapon"
        const std::string baseSuffix = "-base";
        const std::string baseNewSuffix = "-base-new";
        if (auto pos = weaponId.rfind(baseNewSuffix); pos != std::string::npos) {
            weaponId.replace(pos, baseNewSuffix.size(), "-weapon-new");
        } else if (auto pos2 = weaponId.rfind(baseSuffix); pos2 != std::string::npos) {
            weaponId.replace(pos2, baseSuffix.size(), "-weapon");
        }
        towerWeaponObject->SetDrawable(atlasLoader->getImage(weaponId));

        const std::optional<glm::vec2> worldPos = map->gridToWorld(tower.GetGridX(), tower.GetGridY());
        if (!worldPos.has_value()) continue;

        towerBaseObject->m_Transform.scale = {kTowerScale * currentScale, kTowerScale * currentScale};
        towerWeaponObject->m_Transform.scale = {kTowerScale * currentScale, kTowerScale * currentScale};

        towerBaseObject->m_Transform.translation = worldPos.value();
        towerWeaponObject->m_Transform.translation = worldPos.value();
        towerWeaponObject->m_Transform.rotation = tower.GetFacingRotation();
    }
}

void GameSession::updateProjectileDisplay() {
    const std::vector<TowerManager::Projectile>& projectiles = towerManager->getProjectiles();
    const float currentScale = map->getCurrentScale();

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

        projectileObject->m_Transform.scale = {kProjectileScale * currentScale, kProjectileScale * currentScale};

        const std::optional<glm::vec2> worldPos = map->gridToWorld(projectile.x, projectile.y);
        if (!worldPos.has_value()) continue;

        projectileObject->m_Transform.translation = worldPos.value();
    }
}

// ★ 加上 NOLINT 標籤，讓 IDE 忽略 const 轉換警告
// NOLINTNEXTLINE(readability-make-member-function-const)
void GameSession::updateHudDisplay() {
    if (!towerHpText || !goldText || !towerHpIconObject || !goldIconObject || !towerHpTextObject || !goldTextObject) return;

    towerHpText->SetText("基地生命: " + std::to_string(baseHp));
    goldText->SetText("金幣: " + std::to_string(gold));
    const int displayWave = (waveCount % static_cast<int>(baseSpawnSchedule.size())) + 1;
    const std::string loopStr = loopCount > 0 ? " (循環 " + std::to_string(loopCount + 1) + ")" : "";
    waveText->SetText("波次: " + std::to_string(displayWave) + loopStr);

    const std::shared_ptr<Core::Context> context = Core::Context::GetInstance();
    const float halfWindowWidth = static_cast<float>(context->GetWindowWidth()) * 0.5F;
    const float halfWindowHeight = static_cast<float>(context->GetWindowHeight()) * 0.5F;

    const glm::vec2 towerIconSize = atlasLoader->getImage("icon-heart")->GetSize();
    const glm::vec2 goldIconSize = atlasLoader->getImage("icon-coins")->GetSize();
    const glm::vec2 waveIconSize = atlasLoader->getImage("icon-flag")->GetSize();
    const glm::vec2 towerTextSize = towerHpText->GetSize();
    const glm::vec2 goldTextSize = goldText->GetSize();
    const glm::vec2 waveTextSize = waveText->GetSize();

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

    // 置中顯示波次
    waveTextObject->m_Transform.translation = {
        0.0F,
        topY - waveTextSize.y * 0.5F
    };
    waveIconObject->m_Transform.translation = {
        waveTextObject->m_Transform.translation.x - waveTextSize.x * 0.5F - kHudGap - waveIconSize.x * kHudIconScale * 0.5F,
        topY - waveIconSize.y * kHudIconScale * 0.5F
    };
}

void GameSession::spawnDebugEnemy(EnemyTypeId enemyTypeId, const std::vector<int>& spawnPointIndices) const {
    const EnemyTypeConfig& config = getEnemyTypeConfig(enemyTypeId);
    enemyManager->spawnEnemiesAt(spawnPointIndices, config.speed, config.moveType, config.maxHealth, config.damageToBase, config.rewardGold, config.spriteId);
}

// -------------------- 無限循環邏輯 --------------------
void GameSession::beginNextLoop() {
    loopCount += 1;

    // 強化倍率：每循環一次血量 ×1.2、速度 ×1.1
    static constexpr float kHpScale   = 1.2F;
    static constexpr float kSpdScale  = 1.1F;

    // 根據累積循環次數計算倍率（從原始配置重新計算，避免浮點誤差累積）
    float hpMult  = 1.0F;
    float spdMult = 1.0F;
    for (int i = 0; i < loopCount; ++i) {
        hpMult  *= kHpScale;
        spdMult *= kSpdScale;
    }

    // 從 baseSpawnSchedule 重新生成強化後的 spawnSchedule
    spawnSchedule = baseSpawnSchedule;
    for (WaveConfig& wave : spawnSchedule) {
        for (SpawnGroup& group : wave.groups) {
            group.hpMultiplier  = hpMult;  // 血量 ×1.2^loopCount
            group.spdMultiplier = spdMult; // 速度 ×1.1^loopCount
        }
    }

    // 重置波次計數，開始新一輪
    waveCount   = 0;
    groupIndex  = 0;
    groupSpawned = 0;
    waveTimer   = 0.0F;
    groupTimer  = 0.0F;

    LOG_INFO("[Session] 進入第 {} 輪循環：HP ×{:.2f}, SPD ×{:.2f}", loopCount + 1, hpMult, spdMult);
}
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
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
      levelNumber(levelNumber), baseHp(0), gold(0), waveCount(0), groupIndex(0),
      groupSpawned(0), loopCount(0), levelCompleted(false) {

    // 先載入圖集，再建立地圖。
    atlasLoader = std::make_unique<AtlasLoader>();
    atlasLoader->loadAtlas("assets/combined.atlas");

    // 讀取單局資料
    const LevelConfig& level = getLevelConfig(levelNumber);

    // 建立地圖與敵人管理器，並傳入 atlasLoader 參考（共用資源）。
    map = std::make_unique<GridMap>(level.mapPath, *atlasLoader);
    enemyManager = std::make_unique<EnemyManager>(*map, *atlasLoader);
    towerManager = std::make_unique<TowerManager>(*map);

    // 預載所有敵人圖片，避免第一次生成敵人時才讀圖造成卡頓。
    static constexpr std::array<EnemyTypeId, 18> preloadEnemyTypes = {
        EnemyTypeId::Regular,
        EnemyTypeId::Fast,
        EnemyTypeId::Strong,
        EnemyTypeId::Heli,
        EnemyTypeId::Jet,
        EnemyTypeId::Armored,
        EnemyTypeId::Healer,
        EnemyTypeId::Toxic,
        EnemyTypeId::Icy,
        EnemyTypeId::Fighter,
        // EnemyTypeId::Mage,
        // EnemyTypeId::Camole,
        // EnemyTypeId::Drone,
        // EnemyTypeId::Boss,
        // EnemyTypeId::BossHealer,
        // EnemyTypeId::BossArmored,
        // EnemyTypeId::Queen,
        // EnemyTypeId::Tank
    };
    for (EnemyTypeId enemyTypeId : preloadEnemyTypes) {
        const EnemyTypeConfig& config = getEnemyTypeConfig(enemyTypeId);
        (void)atlasLoader->getImage(config.spriteId);
    }

    // 預載塔、子彈與 HUD 會用到的圖片。
    static constexpr std::array<std::string_view, 15> preloadTowerSprites = {
        "tower-basic-base",
        "tower-basic-weapon",
        "tower-sniper-base-new",
        "tower-sniper-weapon-new",
        "tower-cannon-base-new",
        "tower-cannon-weapon-new",
        "projectile-basic",
        "icon-tower",
        "icon-tower-top",
        "aura-range",
        "game-ui-health-icon",
        "game-ui-coin-icon",
        "tile-type-platform",
        "build-selection",
        "icon-step-forward"
    };
    for (std::string_view spriteId : preloadTowerSprites) {
        (void)atlasLoader->getImage(spriteId);
    }

    // 保存第一輪設定，之後無限循環會從這份原始資料重新計算倍率。
    initBaseHp = level.baseHp;
    initGold = level.startingGold;
    baseSpawnSchedule = level.waves; // 保存原始配置，供後續循環計算用
    spawnSchedule = level.waves;
    initSession();

    // 建立基地血量圖示與數字。
    towerHpIconObject = std::make_shared<Util::GameObject>();
    towerHpIconObject->SetDrawable(atlasLoader->getImage("game-ui-health-icon"));
    towerHpIconObject->m_Transform.scale = {kHudIconScale, kHudIconScale};
    towerHpIconObject->SetZIndex(kHudZIndex);

    towerHpText = std::make_shared<Util::Text>(
        kHudFontPath, kHudFontSize, "0", Util::Color::FromRGB(255, 96, 96)
    );
    towerHpTextObject = std::make_shared<Util::GameObject>();
    towerHpTextObject->SetDrawable(towerHpText);
    towerHpTextObject->SetZIndex(kHudZIndex);

    // 建立金幣圖示與數字。
    goldIconObject = std::make_shared<Util::GameObject>();
    goldIconObject->SetDrawable(atlasLoader->getImage("game-ui-coin-icon"));
    goldIconObject->m_Transform.scale = {kHudIconScale, kHudIconScale};
    goldIconObject->SetZIndex(kHudZIndex);

    goldText = std::make_shared<Util::Text>(
        kHudFontPath, kHudFontSize, "0", Util::Color::FromRGB(255, 216, 80)
    );
    goldTextObject = std::make_shared<Util::GameObject>();
    goldTextObject->SetDrawable(goldText);
    goldTextObject->SetZIndex(kHudZIndex);

    // 建立波次旗幟圖示與數字。
    waveIconObject = std::make_shared<Util::GameObject>();
    waveIconObject->SetDrawable(atlasLoader->getImage("icon-flag"));
    waveIconObject->m_Transform.scale = {kHudIconScale, kHudIconScale};
    waveIconObject->SetZIndex(kHudZIndex);

    waveText = std::make_shared<Util::Text>(
        kHudFontPath, kHudFontSize, "0", Util::Color::FromRGB(255, 255, 255)
    );
    waveTextObject = std::make_shared<Util::GameObject>();
    waveTextObject->SetDrawable(waveText);
    waveTextObject->SetZIndex(kHudZIndex);

    // 把 HUD 物件加入 renderer，之後每幀只更新數字與位置。
    hudRoot.AddChild(towerHpIconObject);
    hudRoot.AddChild(towerHpTextObject);
    hudRoot.AddChild(goldIconObject);
    hudRoot.AddChild(goldTextObject);
    hudRoot.AddChild(waveIconObject);
    hudRoot.AddChild(waveTextObject);
    updateHudDisplay();

    // 第二關以後完成第一輪時顯示的跳關按鈕。
    nextLevelButtonObject = std::make_shared<Util::GameObject>();
    nextLevelButtonObject->SetDrawable(atlasLoader->getImage("icon-step-forward"));
    nextLevelButtonObject->SetZIndex(kHudZIndex + 1.0F);
    nextLevelButtonRoot.AddChild(nextLevelButtonObject);
    updateNextLevelButtonDisplay();

    // 背景改為 Infinitode 風格的灰色同色系 #181818。
    glClearColor(24.0F / 255.0F, 24.0F / 255.0F, 24.0F / 255.0F, 1.0F);

    // 建立選塔面板（必須在 atlasLoader 內容完成後）
    m_SelectionPanel = std::make_unique<TowerSelectionPanel>(*atlasLoader);

    // 建立 FPS 顯示，只由遊戲畫面負責更新與繪製。
    m_FpsOverlay = std::make_unique<FpsOverlay>();
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
    // 1) 先取得塔的定義，確認玩家金幣足夠。
    const TowerDef& def = getTowerDef(towerId);
    if (gold < def.buildCost) { return false; }

    // 2) 交給 TowerManager 檢查格子是否合法並實際建塔。
    if (!towerManager->placeTower(gridX, gridY, towerId)) { return false; }

    // 3) 建塔成功後扣錢，並刷新塔與 HUD 顯示。
    gold -= def.buildCost;
    updateTowerDisplay();
    updateHudDisplay();
    return true;
}

bool GameSession::sellTower(int gridX, int gridY) {
    // 1) 先確認指定格子真的有塔。
    auto towerIdOpt = towerManager->getTowerIdAt(gridX, gridY);
    if (!towerIdOpt.has_value()) return false;

    // 2) 根據塔的建造費用計算退還金額。
    const TowerDef& def = getTowerDef(towerIdOpt.value());
    int refund = def.buildCost / 2;

    // 3) 移除成功才加錢，並同步刷新顯示。
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
int GameSession::getLevelNumber() const { return levelNumber; }
bool GameSession::isLevelCompleted() const { return levelCompleted; }

bool GameSession::hitTestNextLevelButton(float screenX, float screenY) const {
    if (!canAdvanceToNextLevel || !nextLevelButtonObject) return false;

    const glm::vec2 center = nextLevelButtonObject->m_Transform.translation;
    const float halfSize = kNextLevelButtonSize * 0.5F;
    return screenX >= center.x - halfSize && screenX <= center.x + halfSize &&
           screenY >= center.y - halfSize && screenY <= center.y + halfSize;
}

void GameSession::advanceToNextLevel() {
    if (canAdvanceToNextLevel) {
        levelCompleted = true;
        canAdvanceToNextLevel = false;
    }
}

// -------------------- 每幀流程 --------------------
void GameSession::update(float deltaTime) {
    update(deltaTime, deltaTime);
}

void GameSession::update(float deltaTime, float rawDeltaTime) {
    if (m_FpsOverlay) { m_FpsOverlay->update(rawDeltaTime); }

    // 1) 暫停時不推進遊戲，只維持畫面物件與 HUD 的位置正確。
    if (!isSessionActive) {
        enemyManager->updateEnemyDisplay(map->getOffsetX(), map->getOffsetY(), map->getCurrentScale());
        updateTowerDisplay();
        updateProjectileDisplay();
        updateHudDisplay();
        return;
    }

    // 2) 推進本局總時間、目前波次時間，以及目前 group 的生成間隔時間。
    timer += deltaTime;
    waveTimer += deltaTime;
    groupTimer += deltaTime;

    // 3) 根據波次計時器決定這一幀是否需要生成敵人。
    dispatchEnemiesByTimer();

    // 4) 更新敵人移動、塔自動攻擊，並把敵人顯示座標同步到目前鏡頭。
    enemyManager->update(deltaTime);
    towerManager->updateAutoAttack(deltaTime, enemyManager->getEnemies());
    enemyManager->updateEnemyDisplay(map->getOffsetX(), map->getOffsetY(), map->getCurrentScale());

    // 5) 結算本幀死亡敵人與抵達終點的敵人。
    const EnemyManager::FrameResolveResult frameResult = enemyManager->resolveAndRemoveDeadAndReached();
    applyBaseDamage(frameResult.reachedGoalDamage);
    addGold(frameResult.killedRewardGold);

    // 6) 基地血量歸零就暫停本局；若已進入無限循環，死亡後換下一關。
    if (!isBaseAlive()) {
        LOG_INFO("[Session] Game Over");
        pauseSession();
        return;
    }

    // 7) 若目前波次已出完且場上無敵人，就發放過關獎勵並切到下一波。
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
        // 8) 第一關沒有無限循環；其他關卡保留原本無限強化倍率流程。
        if (levelNumber == 1 && hasNextLevel()) {
            levelCompleted = true;
            pauseSession();
            LOG_INFO("[Session] Level {} completed", levelNumber);
        } else {
            beginNextLoop();
        }
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void GameSession::display() {
    // 1) 先畫地圖，作為所有遊戲物件的底層。
    map->displayMap();

    // 2) 更新並繪製塔。
    updateTowerDisplay();
    towerRoot.Update();

    // 3) 更新並繪製子彈。
    updateProjectileDisplay();
    projectileRoot.Update();

    // 4) 繪製敵人。
    enemyManager->display();

    // 5) 最後繪製 HUD，避免被地圖、塔或敵人蓋住。
    updateHudDisplay();
    hudRoot.Update();

    // 6) 選塔面板最後繪製，確保右下角操作介面永遠在最上層。
    if (m_SelectionPanel) { m_SelectionPanel->display(); }

    // 7) FPS 疊圖只在 GameSession 畫面出現。
    if (m_FpsOverlay) { m_FpsOverlay->display(); }

    // 8) 進入無限循環後，顯示可手動前往下一關的按鈕。
    if (canAdvanceToNextLevel) {
        updateNextLevelButtonDisplay();
        nextLevelButtonRoot.Update();
    }
}

void GameSession::moveCamera(float dx, float dy) const {
    // 1) 鏡頭位移只需要交給地圖處理，其他物件會在 display/update 時同步座標。
    map->moveCamera(dx, dy);
}

void GameSession::zoomCamera(float zoomDelta) const {
    // 1) 縮放倍率由地圖統一管理，塔、敵人與子彈顯示會跟著目前 scale 更新。
    map->zoomCamera(zoomDelta);
}

// -------------------- 遊戲流程控制 --------------------
void GameSession::initSession() {
    // 1) 重置本局流程控制狀態。
    isSessionActive = false;
    timer = 0.0F;
    waveTimer = 0.0F;
    groupTimer = 0.0F;

    // 2) 回復關卡起始生命、金幣與波次索引。
    baseHp = initBaseHp;
    gold = initGold;
    waveCount = 0;
    groupIndex = 0;
    groupSpawned = 0;
    levelCompleted = false;
    canAdvanceToNextLevel = false;

    // 3) 回到第一輪，並把 spawnSchedule 還原成原始波次設定。
    loopCount = 0;
    spawnSchedule = baseSpawnSchedule; // 重置回第一輪原始配置

    // 4) 清空場上敵人與塔，避免重新開始時保留上一局狀態。
    enemyManager->getEnemies().clear();
    towerManager->clear();

    // 5) 立即刷新所有畫面物件與 HUD。
    updateTowerDisplay();
    updateProjectileDisplay();
    updateHudDisplay();
    LOG_INFO("[Session] init: baseHp={}, gold={}", baseHp, gold);
}

void GameSession::startSession() { isSessionActive = true; }
void GameSession::pauseSession() { isSessionActive = false; }

void GameSession::dispatchEnemiesByTimer() {
    // 1) 先確認波次合法。
    if (waveCount < 0 || waveCount >= static_cast<int>(spawnSchedule.size())) return;

    const WaveConfig& waveConfig = spawnSchedule[waveCount];
    const int groupCount = static_cast<int>(waveConfig.groups.size());

    // 2) 先等本波 prepTime。
    if (waveTimer >= waveConfig.prepTime) {
        // 3) 只處理目前 groupIndex 這一組（依序跑，不並行）。
        if (groupIndex < groupCount) {
            const SpawnGroup& spawnGroup = waveConfig.groups[groupIndex];
            const EnemyTypeConfig& config = getEnemyTypeConfig(spawnGroup.type);

            // 4) 套用循環強化倍率，讓後續輪次的敵人變快、血量變高。
            const float scaledSpeed  = config.speed     * spawnGroup.spdMultiplier;
            const int   scaledHp     = static_cast<int>(static_cast<float>(config.maxHealth) * spawnGroup.hpMultiplier);

            // 5) 第一隻：等到 prepTime + startDelay。
            if (groupSpawned == 0) {
                if (waveTimer >= waveConfig.prepTime + spawnGroup.startDelay) {
                    enemyManager->spawnEnemiesAt(
                        spawnGroup.spawnPointIndices,
                        scaledSpeed,
                        config.moveType,
                        scaledHp,
                        config.damageToBase,
                        config.rewardGold,
                        config.spriteId
                    );
                    groupSpawned += 1;
                    groupTimer = 0.0F;
                }
            } else {
                // 6) 後續：用 groupTimer + interval 控制每次再生一隻。
                if (spawnGroup.interval <= 0.0F || groupTimer >= spawnGroup.interval) {
                    enemyManager->spawnEnemiesAt(
                        spawnGroup.spawnPointIndices,
                        scaledSpeed,
                        config.moveType,
                        scaledHp,
                        config.damageToBase,
                        config.rewardGold,
                        config.spriteId
                    );
                    groupSpawned += 1;
                    groupTimer = 0.0F;
                }
            }

            // 7) 這組完成就切下一組。
            if (groupSpawned >= spawnGroup.count) {
                groupIndex += 1;
                groupSpawned = 0;
                groupTimer = 0.0F;
            }
        }
    }
}

void GameSession::updateTowerDisplay() {
    // 1) 取得目前所有塔與地圖縮放倍率。
    const std::vector<Tower>& towers = towerManager->getTowers();
    const float currentScale = map->getCurrentScale();

    // 2) 如果塔變多，就補上對應的底座與武器 GameObject。
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

    // 3) 如果塔變少，就移除多出來的顯示物件。
    while (towerBaseObjects.size() > towers.size()) {
        towerRoot.RemoveChild(towerBaseObjects.back());
        towerBaseObjects.pop_back();

        towerRoot.RemoveChild(towerWeaponObjects.back());
        towerWeaponObjects.pop_back();
    }

    // 4) 逐一同步塔圖片、世界座標、縮放與武器朝向。
    for (std::size_t i = 0; i < towers.size(); ++i) {
        const Tower& tower = towers[i];
        const std::shared_ptr<Util::GameObject>& towerBaseObject = towerBaseObjects[i];
        const std::shared_ptr<Util::GameObject>& towerWeaponObject = towerWeaponObjects[i];

        towerBaseObject->SetDrawable(atlasLoader->getImage(tower.GetspriteId()));

        // 5) 尋找對應武器：把 base spriteId 轉成 weapon spriteId。
        std::string weaponId = tower.GetspriteId();
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
    // 1) 取得目前所有子彈與地圖縮放倍率。
    const std::vector<TowerManager::Projectile>& projectiles = towerManager->getProjectiles();
    const float currentScale = map->getCurrentScale();

    // 2) 如果子彈變多，就補上顯示物件。
    while (projectileObjects.size() < projectiles.size()) {
        std::shared_ptr<Util::GameObject> projectileObject = std::make_shared<Util::GameObject>();
        projectileObject->SetZIndex(kProjectileZIndex);
        projectileObject->m_Transform.scale = {kProjectileScale, kProjectileScale};
        projectileObject->SetDrawable(atlasLoader->getImage("projectile-basic"));
        projectileRoot.AddChild(projectileObject);
        projectileObjects.push_back(projectileObject);
    }

    // 3) 如果子彈變少，就移除多出來的顯示物件。
    while (projectileObjects.size() > projectiles.size()) {
        projectileRoot.RemoveChild(projectileObjects.back());
        projectileObjects.pop_back();
    }

    // 4) 逐一同步子彈縮放與世界座標。
    for (std::size_t i = 0; i < projectiles.size(); ++i) {
        const TowerManager::Projectile& projectile = projectiles[i];
        const std::shared_ptr<Util::GameObject>& projectileObject = projectileObjects[i];

        projectileObject->m_Transform.scale = {kProjectileScale * currentScale, kProjectileScale * currentScale};

        const std::optional<glm::vec2> worldPos = map->gridToWorld(projectile.x, projectile.y);
        if (!worldPos.has_value()) continue;

        projectileObject->m_Transform.translation = worldPos.value();
    }
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void GameSession::updateHudDisplay() {
    // 1) 如果 HUD 物件尚未建立完成，就先不更新。
    if (!towerHpText || !goldText || !waveText ||
        !towerHpIconObject || !goldIconObject || !waveIconObject ||
        !towerHpTextObject || !goldTextObject || !waveTextObject) return;

    // 2) 更新 HUD 數字，不顯示中文 label。
    towerHpText->SetText(std::to_string(baseHp));
    goldText->SetText(std::to_string(gold));
    const int displayWave = (waveCount % static_cast<int>(baseSpawnSchedule.size())) + 1;
    waveText->SetText(std::to_string(loopCount + 1) + "-" + std::to_string(displayWave));

    // 3) 取得目前視窗大小，HUD 使用螢幕中心座標系定位。
    const std::shared_ptr<Core::Context> context = Core::Context::GetInstance();
    const float halfWindowWidth = static_cast<float>(context->GetWindowWidth()) * 0.5F;
    const float halfWindowHeight = static_cast<float>(context->GetWindowHeight()) * 0.5F;

    // 4) 取得圖片與文字尺寸，後面用來讓 icon 和數字對齊。
    const glm::vec2 towerIconSize = atlasLoader->getImage("game-ui-health-icon")->GetSize();
    const glm::vec2 goldIconSize = atlasLoader->getImage("game-ui-coin-icon")->GetSize();
    const glm::vec2 waveIconSize = atlasLoader->getImage("icon-flag")->GetSize();
    const glm::vec2 towerTextSize = towerHpText->GetSize();
    const glm::vec2 goldTextSize = goldText->GetSize();
    const glm::vec2 waveTextSize = waveText->GetSize();

    const float hudSideMargin = 32.0F;
    const float hudCenterY = halfWindowHeight - 38.0F;
    const float towerIconWidth = towerIconSize.x * kHudIconScale;
    const float goldIconWidth = goldIconSize.x * kHudIconScale;
    const float waveIconWidth = waveIconSize.x * kHudIconScale;
    const float waveGroupWidth = waveIconWidth + kHudGap + waveTextSize.x;
    const float goldGroupWidth = goldIconWidth + kHudGap + goldTextSize.x;

    // 5) 左上角顯示基地生命 icon + 數字。
    const float towerGroupLeft = -halfWindowWidth + hudSideMargin;
    towerHpIconObject->m_Transform.translation = {
        towerGroupLeft + towerIconWidth * 0.5F,
        hudCenterY
    };
    towerHpTextObject->m_Transform.translation = {
        towerHpIconObject->m_Transform.translation.x + towerIconWidth * 0.5F + kHudGap + towerTextSize.x * 0.5F,
        hudCenterY
    };

    // 6) 上方置中顯示波次 icon + 數字。
    const float waveGroupLeft = -waveGroupWidth * 0.5F;
    waveIconObject->m_Transform.translation = {
        waveGroupLeft + waveIconWidth * 0.5F,
        hudCenterY
    };
    waveTextObject->m_Transform.translation = {
        waveIconObject->m_Transform.translation.x + waveIconWidth * 0.5F + kHudGap + waveTextSize.x * 0.5F,
        hudCenterY
    };

    // 7) 右上角顯示金幣 icon + 數字。
    const float goldGroupLeft = halfWindowWidth - hudSideMargin - goldGroupWidth;
    goldIconObject->m_Transform.translation = {
        goldGroupLeft + goldIconWidth * 0.5F,
        hudCenterY
    };
    goldTextObject->m_Transform.translation = {
        goldIconObject->m_Transform.translation.x + goldIconWidth * 0.5F + kHudGap + goldTextSize.x * 0.5F,
        hudCenterY
    };
}

void GameSession::updateNextLevelButtonDisplay() {
    if (!nextLevelButtonObject) return;

    const std::shared_ptr<Core::Context> context = Core::Context::GetInstance();
    const float halfWindowWidth = static_cast<float>(context->GetWindowWidth()) * 0.5F;
    const float halfWindowHeight = static_cast<float>(context->GetWindowHeight()) * 0.5F;
    const glm::vec2 iconSize = atlasLoader->getImage("icon-step-forward")->GetSize();

    nextLevelButtonObject->m_Transform.translation = {
        halfWindowWidth - kNextLevelButtonMargin - kNextLevelButtonSize * 0.5F,
        -halfWindowHeight + kNextLevelButtonBottomOffset + kNextLevelButtonSize * 0.5F
    };
    nextLevelButtonObject->m_Transform.scale = {
        kNextLevelButtonSize / iconSize.x,
        kNextLevelButtonSize / iconSize.y
    };
}

void GameSession::spawnDebugEnemy(EnemyTypeId enemyTypeId, const std::vector<int>& spawnPointIndices) const {
    // 1) 取得指定敵人類型的設定。
    const EnemyTypeConfig& config = getEnemyTypeConfig(enemyTypeId);

    // 2) 直接在指定出生點生成敵人，方便測試地圖路徑與戰鬥。
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

bool GameSession::hasNextLevel() const {
    const int nextLevelNumber = levelNumber + 1;
    for (const LevelConfig& levelConfig : getAllLevelConfigs()) {
        if (levelConfig.levelNumber == nextLevelNumber) {
            return true;
        }
    }
    return false;
}

void GameSession::showPassedLevelButton() {
    if (hasNextLevel()) {
        canAdvanceToNextLevel = true;
    }
}

// -------------------- 無限循環邏輯 --------------------
void GameSession::beginNextLoop() {
    // 1) 完成一輪後，累加循環次數。
    loopCount += 1;

    // 判斷是否達成完跳關條件，達成就顯示跳關按鈕。
    if (loopCount == 1 && hasNextLevel()) {
        canAdvanceToNextLevel = true;
    }

    // 2) 設定每循環一次的強化倍率：血量 x1.2、速度 x1.1。
    static constexpr float kHpScale   = 1.2F;
    static constexpr float kSpdScale  = 1.1F;

    // 3) 根據累積循環次數計算倍率，從 1.0 重新乘起避免誤差累積。
    float hpMult  = 1.0F;
    float spdMult = 1.0F;
    for (int i = 0; i < loopCount; ++i) {
        hpMult  *= kHpScale;
        spdMult *= kSpdScale;
    }

    // 4) 從原始波次重新生成本輪 spawnSchedule，並套用本輪倍率。
    spawnSchedule = baseSpawnSchedule;
    for (WaveConfig& wave : spawnSchedule) {
        for (SpawnGroup& group : wave.groups) {
            group.hpMultiplier  = hpMult;  // 血量 ×1.2^loopCount
            group.spdMultiplier = spdMult; // 速度 ×1.1^loopCount
        }
    }

    // 5) 重置波次與 group 計時狀態，從新一輪第一波開始。
    waveCount   = 0;
    groupIndex  = 0;
    groupSpawned = 0;
    waveTimer   = 0.0F;
    groupTimer  = 0.0F;

    LOG_INFO("[Session] 進入第 {} 輪循環：HP ×{:.2f}, SPD ×{:.2f}", loopCount + 1, hpMult, spdMult);
}

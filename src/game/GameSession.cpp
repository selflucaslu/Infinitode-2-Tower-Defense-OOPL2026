#include "game/GameSession.hpp"

// -------------------- 建立單局 --------------------
GameSession::GameSession(std::string_view mapFilePath, int initialBaseHp, int initialWave)
    : atlasLoader(std::make_unique<AtlasLoader>()),
      baseHp(initialBaseHp),
      wave(initialWave) {

    // 背景改為 Infinitode 風格的灰色同色系 #181818。
    glClearColor(24.0F / 255.0F, 24.0F / 255.0F, 24.0F / 255.0F, 1.0F);

    // 最小流程：先載入圖集，再建立地圖。
    atlasLoader->loadAtlas("assets/combined.atlas");
    // 先建立地圖，再建立 EnemyManager（EnemyManager 需要引用 map）。
    map = std::make_unique<GridMap>(mapFilePath, *atlasLoader);
    // EnemyManager 共用本局 atlasLoader，不重複持有資源。
    enemyManager = std::make_unique<EnemyManager>(*map, *atlasLoader);
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

// -------------------- 基地血量 --------------------
int GameSession::getBaseHp() const {
    return baseHp;
}

void GameSession::setBaseHp(int newBaseHp) {
    baseHp = newBaseHp;
}

void GameSession::applyBaseDamage(int damage) {
    baseHp -= damage;
    if (baseHp < 0) {
        baseHp = 0;
    }
}

bool GameSession::isBaseAlive() const {
    return baseHp > 0;
}

// -------------------- 波次 --------------------
int GameSession::getWave() const {
    return wave;
}

void GameSession::setWave(int newWave) {
    wave = newWave;
}

void GameSession::nextWave() {
    wave += 1;
}

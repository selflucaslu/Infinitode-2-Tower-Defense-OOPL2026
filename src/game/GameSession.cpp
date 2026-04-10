#include "game/GameSession.hpp"

#include "enemy/EnemyManager.hpp"

// -------------------- 建立單局 --------------------
GameSession::GameSession(std::string_view mapFilePath, int initialBaseHp, int initialWave)
    : atlasLoader(std::make_shared<AtlasLoader>()),
      map(nullptr),
      enemyManager(nullptr),
      baseHp(initialBaseHp),
      wave(initialWave) {

    // 背景改為 Infinitode 風格的灰色同色系 #181818。
    glClearColor(24.0F / 255.0F, 24.0F / 255.0F, 24.0F / 255.0F, 1.0F);

    // 最小流程：先載入圖集，再建立地圖。
    atlasLoader->loadAtlas("assets/combined.atlas");
    map = std::make_unique<GridMap>(mapFilePath, atlasLoader);
    enemyManager = std::make_unique<EnemyManager>(*map, atlasLoader);

    // 初始化 TowerManager
    towerManager = std::make_unique<TowerManager>(*map);
}

GameSession::~GameSession() = default;

// -------------------- 地圖存取 --------------------
GridMap& GameSession::getMap() {
    return *map;
}

const GridMap& GameSession::getMap() const {
    return *map;
}

std::shared_ptr<AtlasLoader> GameSession::getAtlasLoader() const {
    return atlasLoader;
}

EnemyManager& GameSession::getEnemyManager() {
    return *enemyManager;
}

const EnemyManager& GameSession::getEnemyManager() const {
    return *enemyManager;
}

void GameSession::update(float deltaTimeSec) {
    enemyManager->update(deltaTimeSec);
}

void GameSession::moveCamera(float dx, float dy) {
    map->moveCamera(dx, dy);
    enemyManager->moveCamera(dx, dy);
}

void GameSession::display() {
    map->displayMap();
    enemyManager->displayEnemies();
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
TowerManager& GameSession::getTowerManager() {
    return *towerManager;
}
#include "game/GameSession.hpp"

// -------------------- 建立單局 --------------------
GameSession::GameSession(std::string_view mapFilePath, int initialBaseHp, int initialWave)
    : atlasLoader(std::make_shared<AtlasLoader>()),
      map(nullptr),
      baseHp(initialBaseHp),
      wave(initialWave) {
    // 最小流程：先載入圖集，再建立地圖。
    atlasLoader->loadAtlas("assets/combined.atlas");
    map = std::make_unique<GridMap>(mapFilePath, atlasLoader);
}

// -------------------- 地圖存取 --------------------
GridMap& GameSession::getMap() {
    return *map;
}

const GridMap& GameSession::getMap() const {
    return *map;
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

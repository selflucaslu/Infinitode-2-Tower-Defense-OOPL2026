#include "tower/TowerManager.hpp"
#include <algorithm>

TowerManager::TowerManager(const GridMap& map)
    : map(map), autoAttackEnabled(true) {
}

bool TowerManager::placeTower(int gridX, int gridY, std::string_view spriteId) {
    // 檢查範圍
    if (gridX < 0 || gridX >= map.getMapWidth() || gridY < 0 || gridY >= map.getMapHeight()) {
        return false;
    }
    // 檢查是否為平台 (Platform)
    if (!map.canBuildTower(gridX, gridY)) {
        return false;
    }
    // 檢查是否已經建了塔
    if (hasTower(gridX, gridY)) {
        return false;
    }
    
    // 通過判定，邏輯上新增一座塔
    towers.emplace_back(std::string(spriteId), gridX, gridY);
    return true;
}

bool TowerManager::removeTower(int gridX, int gridY) {
    auto it = std::find_if(towers.begin(), towers.end(),
        [gridX, gridY](const Tower& t) { return t.GetGridX() == gridX && t.GetGridY() == gridY; });
    
    if (it != towers.end()) {
        towers.erase(it);
        return true;
    }
    return false;
}

bool TowerManager::hasTower(int gridX, int gridY) const {
    return findTowerIndex(gridX, gridY).has_value();
}

std::optional<std::size_t> TowerManager::findTowerIndex(int gridX, int gridY) const {
    for (std::size_t i = 0; i < towers.size(); ++i) {
        if (towers[i].GetGridX() == gridX && towers[i].GetGridY() == gridY) {
            return i;
        }
    }
    return std::nullopt;
}

// 其他 POC 留空介面可暫不實作
void TowerManager::setAutoAttackEnabled(bool enabled) { autoAttackEnabled = enabled; }
bool TowerManager::getAutoAttackEnabled() const { return autoAttackEnabled; }
void TowerManager::updateAutoAttack(float deltaTime, std::vector<Enemy>& enemies) {
    (void)deltaTime;
    (void)enemies;
}
std::vector<Tower>& TowerManager::getTowers() { return towers; }
const std::vector<Tower>& TowerManager::getTowers() const { return towers; }

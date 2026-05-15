set(SRC_FILES
    App.cpp
    game/GameSession.cpp
    utils/AtlasLoader.cpp
    utils/FpsOverlay.cpp
    utils/TowerSelectionPanel.cpp
    enemy/Enemy.cpp
    enemy/EnemyManager.cpp
    map/GridMap.cpp
    map/Tile.cpp
    tower/Tower.cpp
    tower/AmmoTower.cpp
    tower/AroundSkillTower.cpp
        tower/TowerManager.cpp
)

set(INCLUDE_FILES
    App.hpp
    game/GameSession.hpp
    game/LevelConfig.hpp
    utils/AtlasLoader.hpp
    utils/FpsOverlay.hpp
    utils/TowerSelectionPanel.hpp
    enemy/Enemy.hpp
    enemy/EnemyTypeConfig.hpp
    enemy/EnemyManager.hpp
    map/GridMap.hpp
    map/Tile.hpp
    tower/Tower.hpp
    tower/AmmoTower.hpp
    tower/AroundSkillTower.hpp
        tower/TowerManager.hpp

)

set(TEST_FILES
)

#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "enemy/EnemyManager.hpp"
#include "map/GridMap.hpp"
#include "utils/AtlasLoader.hpp"

#include <memory>

namespace {
std::unique_ptr<GridMap> g_Map;
std::unique_ptr<EnemyManager> g_EnemyManager;
std::shared_ptr<AtlasLoader> g_AtlasLoader;
}

void App::Start() {
    LOG_TRACE("Start");
    g_AtlasLoader = std::make_shared<AtlasLoader>();
    g_AtlasLoader->loadAtlas("assets/combined.atlas");
    g_Map = std::make_unique<GridMap>("assets/maps/map_01.csv", g_AtlasLoader);
    g_EnemyManager = std::make_unique<EnemyManager>(*g_Map, g_AtlasLoader);

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    if (g_Map) {
        constexpr float cameraSpeed = 8.0F;
        float dx = 0.0F;
        float dy = 0.0F;

        if (Util::Input::IsKeyPressed(Util::Keycode::W)) dy -= cameraSpeed;  // 上（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::S)) dy += cameraSpeed;  // 下（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::A)) dx += cameraSpeed;  // 左（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::D)) dx -= cameraSpeed;  // 右（長按連續）

        if (dx != 0.0F || dy != 0.0F) {
            g_Map->moveCamera(dx, dy);
            if (g_EnemyManager) {
                g_EnemyManager->moveCamera(dx, dy);
            }
        }

        if (g_EnemyManager) {
            const float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0F;
            g_EnemyManager->update(deltaTimeSec);
        }

        g_Map->displayMap();
        if (g_EnemyManager) {
            g_EnemyManager->displayEnemies();
        }
    }
    
    // Do not touch the code below as they serve the purpose for
    // closing the window.
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}

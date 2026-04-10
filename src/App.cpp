#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "enemy/EnemyManager.hpp"

#include <memory>

namespace {
std::unique_ptr<EnemyManager> g_EnemyManager;
}

// -------------------- 初始化 --------------------
void App::Start() {
    LOG_TRACE("Start");
    // 建立最簡單單局（地圖 + 基地血量 + 波次）
    m_GameSession = std::make_unique<GameSession>("assets/maps/map_01.csv");

    if (m_GameSession) {
        g_EnemyManager = std::make_unique<EnemyManager>(
            m_GameSession->getMap(),
            m_GameSession->getAtlasLoader()
        );
    }

    m_CurrentState = State::UPDATE;
}

// -------------------- 每幀更新 --------------------
void App::Update() {
    if (m_GameSession) {
        GridMap& map = m_GameSession->getMap();
        constexpr float cameraSpeed = 8.0F;
        float dx = 0.0F;
        float dy = 0.0F;

        if (Util::Input::IsKeyPressed(Util::Keycode::W)) dy -= cameraSpeed;  // 上（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::S)) dy += cameraSpeed;  // 下（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::A)) dx += cameraSpeed;  // 左（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::D)) dx -= cameraSpeed;  // 右（長按連續）

        if (dx != 0.0F || dy != 0.0F) {
            map.moveCamera(dx, dy);
            if (g_EnemyManager) {
                g_EnemyManager->moveCamera(dx, dy);
            }
        }

        if (g_EnemyManager) {
            const float deltaTimeSec = Util::Time::GetDeltaTimeMs() / 1000.0F;
            g_EnemyManager->update(deltaTimeSec);
        }

        map.displayMap();
        if (g_EnemyManager) {
            g_EnemyManager->displayEnemies();
        }
    }

    // ESC 或視窗關閉 -> 進入結束流程
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

// -------------------- 結束 --------------------
void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}

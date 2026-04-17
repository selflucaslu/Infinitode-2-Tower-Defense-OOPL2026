#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

// -------------------- 初始化 --------------------
void App::Start() {
    LOG_TRACE("Start");
    // 建立最簡單單局（地圖 + 基地血量 + 波次）
    m_GameSession = std::make_unique<GameSession>("assets/maps/map_01.csv");

    m_CurrentState = State::UPDATE;
}

// -------------------- 每幀更新 --------------------
void App::Update() {
    if (m_GameSession) {
        // 取出本局地圖與敵人管理器，統一在這裡做每幀推進。
        GridMap& map = m_GameSession->getMap();
        EnemyManager& enemyManager = m_GameSession->getEnemyManager();
        constexpr float cameraSpeed = 8.0F;
        float dx = 0.0F;
        float dy = 0.0F;

        if (Util::Input::IsKeyPressed(Util::Keycode::W)) dy -= cameraSpeed;  // 上（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::S)) dy += cameraSpeed;  // 下（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::A)) dx += cameraSpeed;  // 左（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::D)) dx -= cameraSpeed;  // 右（長按連續）

        // 地圖與敵人共用同一組相機平移，避免畫面脫節。
        if (dx != 0.0F || dy != 0.0F) {
            map.moveCamera(dx, dy);
            enemyManager.moveCamera(dx, dy);
        }

        // 先更新敵人，再繪製地圖（敵人渲染由 EnemyManager::update 內部處理）。
        // 目前沿用最簡單固定時間步進；後續可改成引擎真實 deltaTime。
        constexpr float fixedDeltaTime = 1.0F / 60.0F;
        enemyManager.update(fixedDeltaTime);
        map.displayMap();
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

#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "enemy/EnemyTypeConfig.hpp"
#include "game/LevelConfig.hpp"

#include <algorithm>

// -------------------- 初始化 --------------------
void App::Start() {
    LOG_TRACE("Start");

    // 啟動時先做關卡配置完整性檢查（含 map 路徑存在性）。
    // 若配置有錯，會丟出例外，由 main 的全域 catch 統一記錄並結束。
    (void)getAllLevelConfigs();

    // 建立最簡單單局（地圖 + 基地血量 + 波次）
    m_GameSession = std::make_unique<GameSession>(2);
    m_GameSession->startSession();

    // 建立 FPS 顯示
    m_FpsOverlay = std::make_unique<FpsOverlay>();

    m_CurrentState = State::UPDATE;
}

// -------------------- 每幀更新 --------------------
void App::Update() {
    if (m_GameSession && Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
        const glm::vec2 mouseWorld = Util::Input::GetCursorPosition();
        if (auto grid = m_GameSession->getMap().worldToGrid(mouseWorld)) {
            LOG_DEBUG("Clicked grid: ({}, {})", grid->first, grid->second);
        } else {
            LOG_DEBUG("Clicked outside map");
        }
    }

    if (m_GameSession) {
        const float rawDeltaTime = Util::Time::GetDeltaTimeMs() * 0.001F; // 真實每幀秒數（秒）
        const float simDeltaTime = std::clamp(rawDeltaTime, 0.0F, 0.05F); // 過濾極端值

        constexpr float cameraSpeedPerSecond = 480.0F; // 相機速度定義（每秒多少單位）
        float dx = 0.0F; // 本幀 x 位移量
        float dy = 0.0F; // 本幀 y 位移量

        // 核心公式：本幀位移 = 每秒速度 * 本幀秒數（simDeltaTime）
        // 例：480 * 0.016 ~= 7.68，代表 60 FPS 時每幀移動約 7.68 單位。
        // FPS 降低時，simDeltaTime 變大、每幀位移也變大，最終「每秒移動距離」仍一致。
        if (Util::Input::IsKeyPressed(Util::Keycode::W)) dy -= cameraSpeedPerSecond * simDeltaTime;  // 上（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::S)) dy += cameraSpeedPerSecond * simDeltaTime;  // 下（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::A)) dx += cameraSpeedPerSecond * simDeltaTime;  // 左（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::D)) dx -= cameraSpeedPerSecond * simDeltaTime;  // 右（長按連續）

        // 相機視角移動。
        if (dx != 0.0F || dy != 0.0F) {
            m_GameSession->moveCamera(dx, dy);
        }

        // 測試代碼：按 1 / 2 生成不同敵人。
        // App 只負責輸入，實際生怪交給 GameSession 協調。
        if (Util::Input::IsKeyDown(Util::Keycode::NUM_1)) {
            m_GameSession->spawnDebugEnemy(EnemyTypeId::Regular);
        }
        if (Util::Input::IsKeyDown(Util::Keycode::NUM_2)) {
            m_GameSession->spawnDebugEnemy(EnemyTypeId::Fast);
        }

        // 每幀順序：
        // update：先更新邏輯狀態（敵人位置、路徑推進...）
        // display：再把更新後結果渲染到螢幕
        m_GameSession->update(simDeltaTime);
        m_GameSession->display();
        if (m_FpsOverlay) {
            m_FpsOverlay->update(rawDeltaTime); // 獨立使用 rawDeltaTime
            m_FpsOverlay->display();
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

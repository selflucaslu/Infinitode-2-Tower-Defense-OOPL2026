#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "enemy/EnemyTypeConfig.hpp"

#include <algorithm>

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
        // 1) 取得上一幀到這一幀的實際經過時間（毫秒 -> 秒），並做夾制：
        // 下限 0.0F（避免異常負值）/ 上限 0.05F（避免卡頓瞬移）。
        const float deltaTime = std::clamp(Util::Time::GetDeltaTimeMs() * 0.001F, 0.0F, 0.05F);

        // 相機速度定義為「每秒多少單位」，不是每幀多少單位。
        // 這樣才能搭配 deltaTime 做出 FPS 無關的等速移動。
        constexpr float cameraSpeedPerSecond = 480.0F;
        float dx = 0.0F; // 本幀 x 位移量（由輸入累加）
        float dy = 0.0F; // 本幀 y 位移量（由輸入累加）

        // 核心公式：本幀位移 = 每秒速度 * 本幀秒數（deltaTime）
        // 例：480 * 0.016 ~= 7.68，代表 60 FPS 時每幀移動約 7.68 單位。
        // FPS 降低時，deltaTime 變大、每幀位移也變大，最終「每秒移動距離」仍一致。
        if (Util::Input::IsKeyPressed(Util::Keycode::W)) dy -= cameraSpeedPerSecond * deltaTime;  // 上（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::S)) dy += cameraSpeedPerSecond * deltaTime;  // 下（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::A)) dx += cameraSpeedPerSecond * deltaTime;  // 左（長按連續）
        if (Util::Input::IsKeyPressed(Util::Keycode::D)) dx -= cameraSpeedPerSecond * deltaTime;  // 右（長按連續）

        // 地圖與敵人共用同一組相機平移，避免畫面脫節。
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
        // display：再把更新後結果畫到螢幕
        // 這是最常見、最直覺的遊戲主迴圈流程。
        m_GameSession->update(deltaTime);
        m_GameSession->display();
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

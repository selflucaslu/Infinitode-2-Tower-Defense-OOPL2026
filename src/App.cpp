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
    // 點擊左鍵建塔邏輯 (使用 GameSession 封裝的方法，具備扣除金幣功能)
    if (m_GameSession && Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
        const glm::vec2 mouseWorld = Util::Input::GetCursorPosition();
        if (auto grid = m_GameSession->getMap().worldToGrid(mouseWorld)) {

            // 修正 Clang-Tidy 警告：將變數直接初始化在 if 判斷式中
            if (const bool placed = m_GameSession->placeTower(grid->first, grid->second, "tower-basic"); placed) {
                LOG_INFO("Tower placed at grid ({}, {}), Gold deducted", grid->first, grid->second);
            } else {
                LOG_DEBUG("Cannot place tower at grid ({}, {}) (Insufficient gold or invalid position)", grid->first, grid->second);
            }

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

        // 處理鍵盤 WASD 以及右鍵拖曳移動地圖
        if (Util::Input::IsKeyPressed(Util::Keycode::W)) dy -= cameraSpeedPerSecond * simDeltaTime;
        if (Util::Input::IsKeyPressed(Util::Keycode::S)) dy += cameraSpeedPerSecond * simDeltaTime;
        if (Util::Input::IsKeyPressed(Util::Keycode::A)) dx += cameraSpeedPerSecond * simDeltaTime;
        if (Util::Input::IsKeyPressed(Util::Keycode::D)) dx -= cameraSpeedPerSecond * simDeltaTime;

        // 處理滑鼠右鍵拖曳移動地圖
        glm::vec2 currentMousePos = Util::Input::GetCursorPosition();
        static glm::vec2 lastMousePos = currentMousePos;
        if (Util::Input::IsKeyPressed(Util::Keycode::MOUSE_RB)) {
            dx += (currentMousePos.x - lastMousePos.x);
            dy += (currentMousePos.y - lastMousePos.y);
        }
        lastMousePos = currentMousePos;

        // 執行相機視角移動
        if (dx != 0.0F || dy != 0.0F) {
            m_GameSession->moveCamera(dx, dy);
        }

        // 處理滾輪縮放地圖
        if (Util::Input::IfScroll()) {
            float scrollDelta = Util::Input::GetScrollDistance().y;
            if (scrollDelta != 0.0F) {
                m_GameSession->zoomCamera(scrollDelta);
            }
        }

        // 處理鍵盤按鍵生成測試敵人
        if (Util::Input::IsKeyDown(Util::Keycode::NUM_1)) {
            m_GameSession->spawnDebugEnemy(EnemyTypeId::Regular);
        }
        if (Util::Input::IsKeyDown(Util::Keycode::NUM_2)) {
            m_GameSession->spawnDebugEnemy(EnemyTypeId::Fast);
        }

        // 每幀順序：先更新邏輯狀態，再將結果渲染到螢幕
        m_GameSession->update(simDeltaTime);
        m_GameSession->display();

        if (m_FpsOverlay) {
            m_FpsOverlay->update(rawDeltaTime);
            m_FpsOverlay->display();
        }
    }

    // 當按下 ESC 或視窗關閉時進入結束流程
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

// -------------------- 結束 --------------------
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void App::End() {
    LOG_TRACE("End");
}
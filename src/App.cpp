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
        GridMap& map = m_GameSession->getMap();
        TowerManager& towerMgr = m_GameSession->getTowerManager();

        // 取得目前的滑鼠資訊
        glm::vec2 currentMousePos = Util::Input::GetCursorPosition();
        static glm::vec2 lastMousePos = currentMousePos;

        // 1. WASD 以及 右鍵拖曳移動地圖
        float dx = 0.0F;
        float dy = 0.0F;
        constexpr float cameraSpeed = 8.0F;
        const float rawDeltaTime = Util::Time::GetDeltaTimeMs() * 0.001F; // 真實每幀秒數（秒）
        const float simDeltaTime = std::clamp(rawDeltaTime, 0.0F, 0.05F); // 過濾極端值

        if (Util::Input::IsKeyPressed(Util::Keycode::W)) dy -= cameraSpeed;
        if (Util::Input::IsKeyPressed(Util::Keycode::S)) dy += cameraSpeed;
        if (Util::Input::IsKeyPressed(Util::Keycode::A)) dx += cameraSpeed;
        if (Util::Input::IsKeyPressed(Util::Keycode::D)) dx -= cameraSpeed;

        if (Util::Input::IsKeyPressed(Util::Keycode::MOUSE_RB)) {
            dx += (currentMousePos.x - lastMousePos.x);
            dy += (currentMousePos.y - lastMousePos.y);
        }
        constexpr float cameraSpeedPerSecond = 480.0F; // 相機速度定義（每秒多少單位）

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

        // 2. 滾輪縮放地圖 (假定 Y 軸包含滾輪數值)
        // 加上 IfScroll() 檢查，確保只有滾動發生的「那一幀」才執行
        if (Util::Input::IfScroll()) {
            float scrollDelta = Util::Input::GetScrollDistance().y;
            if (scrollDelta != 0.0F) {
                m_GameSession->zoomCamera(scrollDelta);
            }
        }

        // 3. 滑鼠左鍵點擊建塔
        // 使用 IsKeyDown 來確保點擊只會觸發一次 (而不是按住連續建塔)
        if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            auto [gridX, gridY] = map.ScreenToGrid(currentMousePos.x, currentMousePos.y);

            // 嘗試在邏輯上建立一座塔 (檢查是否有越界、是否可放塔、是否已被佔用)
            if (towerMgr.placeTower(gridX, gridY, "tower-basic")) {
                // 如果邏輯判定成功，就在畫面上新增塔的視覺物件
                map.addTowerVisual(gridX, gridY, "tower-basic");
                LOG_TRACE("成功建立塔於 ({}, {})", gridX, gridY);
            } else {
                LOG_TRACE("無法於該位置建立塔 ({}, {})", gridX, gridY);
            }
        }

        // 更新儲存上幀座標
        lastMousePos = currentMousePos;

        map.displayMap();
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

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

// -------------------- 結束 --------------------
void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}

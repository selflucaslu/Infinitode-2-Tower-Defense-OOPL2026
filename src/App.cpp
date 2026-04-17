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
        GridMap& map = m_GameSession->getMap();
        TowerManager& towerMgr = m_GameSession->getTowerManager();

        // 取得目前的滑鼠資訊
        glm::vec2 currentMousePos = Util::Input::GetCursorPosition();
        static glm::vec2 lastMousePos = currentMousePos;

        // 1. WASD 以及 右鍵拖曳移動地圖
        float dx = 0.0F;
        float dy = 0.0F;
        constexpr float cameraSpeed = 8.0F;

        if (Util::Input::IsKeyPressed(Util::Keycode::W)) dy -= cameraSpeed;
        if (Util::Input::IsKeyPressed(Util::Keycode::S)) dy += cameraSpeed;
        if (Util::Input::IsKeyPressed(Util::Keycode::A)) dx += cameraSpeed;
        if (Util::Input::IsKeyPressed(Util::Keycode::D)) dx -= cameraSpeed;

        if (Util::Input::IsKeyPressed(Util::Keycode::MOUSE_RB)) {
            dx += (currentMousePos.x - lastMousePos.x);
            dy += (currentMousePos.y - lastMousePos.y);
        }

        if (dx != 0.0F || dy != 0.0F) {
            map.moveCamera(dx, dy);
        }

        // 2. 滾輪縮放地圖 (假定 Y 軸包含滾輪數值)
        float scrollDelta = Util::Input::GetScrollDistance().y;
        if (scrollDelta != 0.0F) {
            map.zoomCamera(scrollDelta * 0.1F);
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
    }

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

// -------------------- 結束 --------------------
void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}

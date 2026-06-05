#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "enemy/EnemyTypeConfig.hpp"
#include "game/LevelConfig.hpp"
#include "tower/TowerDef.hpp"

#include <algorithm>

// -------------------- 初始化 --------------------
void App::Start() {
  LOG_TRACE("Start");

  // 啟動時先做關卡配置完整性檢查（含 map 路徑存在性）。
  // 若配置有錯，會丟出例外，由 main 的全域 catch 統一記錄並結束。
  (void)getAllLevelConfigs();

  // MVP 首頁只負責開始遊戲與離開，真正進入遊戲時才建立 GameSession。
  m_Home = std::make_unique<Home>();

  m_CurrentState = State::HOME;
}

// -------------------- 每幀更新 --------------------
void App::Update() {
  if (m_CurrentState == State::HOME) {
    if (m_Home) {
      const HomeAction action = m_Home->update();
      m_Home->display();

      if (action == HomeAction::StartGame) {
        m_GameSession = std::make_unique<GameSession>(5);//更改起點
        m_GameSession->startSession();
        m_CurrentState = State::GAME;
      } else if (action == HomeAction::Quit) {
        m_CurrentState = State::END;
      }
    }

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
      m_CurrentState = State::END;
    }
    return;
  }

  // 切換塔種類（數字鍵 1 / 2 / 3）
  if (Util::Input::IsKeyDown(Util::Keycode::NUM_1)) m_SelectedTower = TowerId::Basic;
  if (Util::Input::IsKeyDown(Util::Keycode::NUM_2)) m_SelectedTower = TowerId::Sniper;
  if (Util::Input::IsKeyDown(Util::Keycode::NUM_3)) m_SelectedTower = TowerId::Cannon;

  // 同步選中塔至 GameSession 選塔面板（含數字鍵切換）
  if (m_GameSession) {
    m_GameSession->setSelectedTower(m_SelectedTower);
  }

  // 點擊左鍵：先判斷是否點擊到選塔面板格子，若是則切換選塔；否則建塔
  if (m_GameSession && Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
    const glm::vec2 mousePos = Util::Input::GetCursorPosition();

    // 先做面板 hitTest（選塔面板使用 OpenGL 螢幕座標，與 GetCursorPosition() 相同）
    if (auto hitTower = m_GameSession->hitTestSelectionPanel(mousePos.x, mousePos.y)) {
      // 點到面板格子 → 切換選塔，不建塔
      m_SelectedTower = *hitTower;
      m_GameSession->setSelectedTower(m_SelectedTower);
      LOG_INFO("Selected tower: {}", getTowerDef(m_SelectedTower).displayName);
    } else if (auto grid = m_GameSession->getMap().worldToGrid(mousePos)) {
      // 點到地圖 → 建塔
      const TowerDef& def = getTowerDef(m_SelectedTower);
      if (const bool placed = m_GameSession->placeTower(
              grid->first, grid->second, m_SelectedTower);
          placed) {
        LOG_INFO("Tower '{}' placed at grid ({}, {}), cost={}",
                 def.displayName, grid->first, grid->second, def.buildCost);
      } else {
        LOG_DEBUG("Cannot place '{}' tower at ({},{}) (gold insufficient or tile invalid)",
                  def.displayName, grid->first, grid->second);
      }
    } else {
      LOG_DEBUG("Clicked outside map and panel");
    }
  }

  // 按下 X 鍵拆除滑鼠位置的塔
  if (m_GameSession && Util::Input::IsKeyDown(Util::Keycode::X)) {
    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    if (auto grid = m_GameSession->getMap().worldToGrid(mousePos)) {
      if (m_GameSession->sellTower(grid->first, grid->second)) {
        LOG_INFO("Tower removed at grid ({}, {}), refund granted", grid->first, grid->second);
      }
    }
  }

  if (m_GameSession) {
    const float rawDeltaTime =
        Util::Time::GetDeltaTimeMs() * 0.001F; // 真實每幀秒數（秒）
    const float simDeltaTime =
        std::clamp(rawDeltaTime, 0.0F, 0.05F); // 過濾極端值

    constexpr float cameraSpeedPerSecond =
        480.0F;      // 相機速度定義（每秒多少單位）
    float dx = 0.0F; // 本幀 x 位移量
    float dy = 0.0F; // 本幀 y 位移量

    // 處理鍵盤 WASD 以及右鍵拖曳移動地圖
    if (Util::Input::IsKeyPressed(Util::Keycode::W))
      dy -= cameraSpeedPerSecond * simDeltaTime;
    if (Util::Input::IsKeyPressed(Util::Keycode::S))
      dy += cameraSpeedPerSecond * simDeltaTime;
    if (Util::Input::IsKeyPressed(Util::Keycode::A))
      dx += cameraSpeedPerSecond * simDeltaTime;
    if (Util::Input::IsKeyPressed(Util::Keycode::D))
      dx -= cameraSpeedPerSecond * simDeltaTime;

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

    // 每幀順序：先更新邏輯狀態，再將結果渲染到螢幕
    m_GameSession->update(simDeltaTime, rawDeltaTime);
    if (m_GameSession->isLevelCompleted()) {
      const int nextLevelNumber = m_GameSession->getLevelNumber() + 1;
      LOG_INFO("Loading level {}", nextLevelNumber);
      m_GameSession = std::make_unique<GameSession>(nextLevelNumber);
      m_SelectedTower = TowerId::Basic;
      m_GameSession->setSelectedTower(m_SelectedTower);
      m_GameSession->startSession();
    }
    m_GameSession->display();
  }

  // 當按下 ESC 或視窗關閉時進入結束流程
  if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
    m_CurrentState = State::END;
  }
}

// -------------------- 結束 --------------------
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void App::End() { LOG_TRACE("End"); }

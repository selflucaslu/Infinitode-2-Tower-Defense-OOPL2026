#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "enemy/EnemyTypeConfig.hpp"
#include "tower/TowerDef.hpp"
#include "ui/Result.hpp"

#include <algorithm>

// -------------------- 分數計算輔助 --------------------
// levelNumber：本關關卡編號（1~5）
// loopCount  ：已完成的循環次數（0 = 第一輪）
// remainHp   ：本關結束時剩餘血量
// remainGold ：本關結束時剩餘金幣
int App::calcLevelScore(int levelNumber, int loopCount,
                        int remainHp, int remainGold) const {
  // 1) 剩餘血量 & 金幣獎勵（每關通用）
  const int hpBonus   = remainHp;
  const int goldBonus = remainGold / 1000;

  // 2) 過關固定獎勵 + 循環加成
  //    { baseBonus, loopBonus } per level
  struct LevelBonus { int base; int loop; };
  static constexpr LevelBonus kTable[5] = {
    {10,  0},   // Level 1
    {15,  5},   // Level 2
    {50, 25},   // Level 3
    {25, 10},   // Level 4
    {60, 20},   // Level 5
  };

  int clearBonus = 0;
  if (levelNumber >= 1 && levelNumber <= 5) {
    const LevelBonus& lb = kTable[levelNumber - 1];
    clearBonus = lb.base + loopCount * lb.loop;
  }

  const int total = hpBonus + goldBonus + clearBonus;
  LOG_INFO("[Score] Level {} loop {} → hp={} gold={} clear={} total={}",
           levelNumber, loopCount, hpBonus, goldBonus, clearBonus, total);
  return total;
}

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
    const bool wasPopupShowing = m_Home && (m_Home->isShowingAbout() || 
                                            m_Home->isShowingHandbook() || 
                                            m_Home->isShowingMusicPlayer() ||
                                            m_Home->isShowingSettings());
    if (m_Home) {
      const HomeAction action = m_Home->update();
      m_Home->display();

      if (action == HomeAction::StartGame) {
        m_GameSession = std::make_unique<GameSession>(1); // 更改起點
        m_GameSession->setCheatMode(m_CheatModeEnabled);
        m_GameSession->startSession();
        m_CurrentState = State::GAME;
      } else if (action == HomeAction::Quit) {
        m_CurrentState = State::END;
      }
    }

    if (!wasPopupShowing) {
      if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
        m_CurrentState = State::END;
      }
    }
    return;
  }

  if (m_CurrentState == State::RESULT) {
    if (m_Result) {
      const ResultAction action = m_Result->update();
      m_Result->display();

      if (action == ResultAction::BackToHome) {
        m_Result.reset();
        m_GameSession.reset();
        m_Score = 0;  // 回首頁時重置分數
        m_Home = std::make_unique<Home>();
        m_CurrentState = State::HOME;
        return;
      }
    }

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
      m_CurrentState = State::END;
    }
    return;
  }

  // 顯示和關閉設定頁面 (Key M)
  if (m_GameSession && Util::Input::IsKeyDown(Util::Keycode::M)) {
    m_GameSession->toggleSettings();
  }

  if (m_GameSession && m_GameSession->isShowingSettings()) {
    m_GameSession->updateSettingsInput();

    if (Util::Input::IfExit()) {
      m_CurrentState = State::END;
      return;
    }

    // 依然渲染，但 simulation deltaTime 為 0 以暫停遊戲
    const float rawDeltaTime = Util::Time::GetDeltaTimeMs() * 0.001F;
    m_GameSession->update(0.0F, rawDeltaTime);
    m_GameSession->display();
    return;
  }

  // 切換作弊模式 (Key F1)
  if (Util::Input::IsKeyDown(Util::Keycode::F1)) {
    m_CheatModeEnabled = !m_CheatModeEnabled;
    if (m_GameSession) {
      m_GameSession->setCheatMode(m_CheatModeEnabled);
    }
    LOG_INFO("Cheat Mode toggled: {}", m_CheatModeEnabled ? "ENABLED" : "DISABLED");
  }

  // 顯示跳關按鈕（測試用，正式版可刪除）
  if (m_CheatModeEnabled && m_GameSession && Util::Input::IsKeyDown(Util::Keycode::P))
    m_GameSession->showPassedLevelButton();

  // 切換塔種類（數字鍵 1 / 2 / 3）
  if (Util::Input::IsKeyDown(Util::Keycode::NUM_1))
    m_SelectedTower = TowerId::Basic;
  if (Util::Input::IsKeyDown(Util::Keycode::NUM_2))
    m_SelectedTower = TowerId::Sniper;
  if (Util::Input::IsKeyDown(Util::Keycode::NUM_3))
    m_SelectedTower = TowerId::Cannon;

  // 同步選中塔至 GameSession 選塔面板（含數字鍵切換）
  if (m_GameSession) {
    m_GameSession->setSelectedTower(m_SelectedTower);
  }

  // 點擊左鍵：先判斷是否點擊到選塔面板格子，若是則切換選塔；否則建塔
  if (m_GameSession && Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
    const glm::vec2 mousePos = Util::Input::GetCursorPosition();

    if (m_GameSession->hitTestSpeedIcon(mousePos.x, mousePos.y)) {
      // 點到速度圖示，不進行建塔或按鈕觸發
    } else if (m_GameSession->hitTestNextLevelButton(mousePos.x, mousePos.y)) {
      m_GameSession->advanceToNextLevel();
      LOG_INFO("Advancing to next level");
    } else if (auto hitTower = m_GameSession->hitTestSelectionPanel(
                   mousePos.x, mousePos.y)) {
      // 點到面板格子 → 切換選塔，不建塔
      m_SelectedTower = *hitTower;
      m_GameSession->setSelectedTower(m_SelectedTower);
      LOG_INFO("Selected tower: {}", getTowerDef(m_SelectedTower).displayName);
    } else if (auto grid = m_GameSession->getMap().worldToGrid(mousePos)) {
      // 點到地圖 → 建塔
      const TowerDef &def = getTowerDef(m_SelectedTower);
      if (const bool placed = m_GameSession->placeTower(
              grid->first, grid->second, m_SelectedTower);
          placed) {
        LOG_INFO("Tower '{}' placed at grid ({}, {}), cost={}", def.displayName,
                 grid->first, grid->second, def.buildCost);
      } else {
        LOG_DEBUG("Cannot place '{}' tower at ({},{}) (gold insufficient or "
                  "tile invalid)",
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
        LOG_INFO("Tower removed at grid ({}, {}), refund granted", grid->first,
                 grid->second);
      }
    }
  }

  if (m_GameSession) {
    const float rawDeltaTime =
        Util::Time::GetDeltaTimeMs() * 0.001F; // 真實每幀秒數（秒）
    const float baseSimDeltaTime =
        std::clamp(rawDeltaTime, 0.0F, 0.05F); // 過濾極端值

    // 處理滑鼠點擊/長按速度圖示 (長按暫停，單擊切換 1x->2x->3x)
    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
      if (m_GameSession->hitTestSpeedIcon(mousePos.x, mousePos.y)) {
        m_SpeedIconPressed = true;
        m_SpeedIconPressTime = 0.0F;
        m_SpeedIconLongPressedTriggered = false;
      }
    }

    if (m_SpeedIconPressed) {
      if (Util::Input::IsKeyPressed(Util::Keycode::MOUSE_LB)) {
        m_SpeedIconPressTime += rawDeltaTime;
        if (!m_SpeedIconLongPressedTriggered && m_SpeedIconPressTime >= 0.5F) {
          m_GameSession->togglePause();
          m_SpeedIconLongPressedTriggered = true;
          LOG_INFO("Speed Icon Long Pressed - Pause toggled. State: {}", m_GameSession->isSpeedPaused() ? "PAUSED" : "RUNNING");
        }
      }

      if (Util::Input::IsKeyUp(Util::Keycode::MOUSE_LB)) {
        if (!m_SpeedIconLongPressedTriggered) {
          m_GameSession->cycleSpeed();
          LOG_INFO("Speed Icon Clicked - Speed cycled to {}x", m_GameSession->getSpeedGear());
        }
        m_SpeedIconPressed = false;
      }
    }

    // 處理鍵盤 J/K/L 熱鍵
    if (!m_GameSession->isShowingSettings()) {
      if (Util::Input::IsKeyDown(Util::Keycode::J)) {
        m_GameSession->decelerateSpeed();
        LOG_INFO("Speed decelerated to {}x (Paused: {})", m_GameSession->getSpeedGear(), m_GameSession->isSpeedPaused() ? "YES" : "NO");
      }
      if (Util::Input::IsKeyDown(Util::Keycode::K)) {
        m_GameSession->togglePause();
        LOG_INFO("Pause toggled via key K. (Paused: {})", m_GameSession->isSpeedPaused() ? "YES" : "NO");
      }
      if (Util::Input::IsKeyDown(Util::Keycode::L)) {
        m_GameSession->accelerateSpeed();
        LOG_INFO("Speed accelerated to {}x (Paused: {})", m_GameSession->getSpeedGear(), m_GameSession->isSpeedPaused() ? "YES" : "NO");
      }
    }

    float simDeltaTime = baseSimDeltaTime;
    if (m_GameSession->isSpeedPaused()) {
      simDeltaTime = 0.0F;
    } else {
      simDeltaTime *= static_cast<float>(m_GameSession->getSpeedGear());
    }

    // 處理作弊按鍵
    if (m_CheatModeEnabled) {
      // H: 加 1 條命
      if (Util::Input::IsKeyDown(Util::Keycode::H)) {
        m_GameSession->setBaseHp(m_GameSession->getBaseHp() + 1);
        LOG_INFO("Cheat: HP increased to {}", m_GameSession->getBaseHp());
      }
      // G: 加 100 金幣
      if (Util::Input::IsKeyDown(Util::Keycode::G)) {
        m_GameSession->addGold(100);
        LOG_INFO("Cheat: Gold increased to {}", m_GameSession->getGold());
      }
    }

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

    // 處理滑鼠右鍵拖曳與單擊拆塔
    glm::vec2 currentMousePos = Util::Input::GetCursorPosition();
    static glm::vec2 lastMousePos = currentMousePos;
    static bool isRightClickPressed = false;
    static glm::vec2 rightClickStartPos = {0.0F, 0.0F};

    if (Util::Input::IsKeyDown(Util::Keycode::MOUSE_RB)) {
      isRightClickPressed = true;
      rightClickStartPos = currentMousePos;
    }

    if (Util::Input::IsKeyPressed(Util::Keycode::MOUSE_RB)) {
      dx += (currentMousePos.x - lastMousePos.x);
      dy += (currentMousePos.y - lastMousePos.y);
    }

    if (isRightClickPressed && Util::Input::IsKeyUp(Util::Keycode::MOUSE_RB)) {
      float dist = glm::distance(currentMousePos, rightClickStartPos);
      if (dist < 8.0F) {
        if (!m_GameSession->hitTestSelectionPanel(currentMousePos.x, currentMousePos.y) &&
            !m_GameSession->hitTestNextLevelButton(currentMousePos.x, currentMousePos.y)) {
          if (auto grid = m_GameSession->getMap().worldToGrid(currentMousePos)) {
            if (m_GameSession->sellTower(grid->first, grid->second)) {
              LOG_INFO("Tower removed at grid ({}, {}), refund granted via right click", grid->first, grid->second);
            }
          }
        }
      }
      isRightClickPressed = false;
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

    // 處理鍵盤 Q/E 縮放 (E: 放大, Q: 縮小)
    static float zoomTimer = 0.0f;
    zoomTimer += simDeltaTime;
    if (zoomTimer >= 0.03F) {
      if (Util::Input::IsKeyPressed(Util::Keycode::E)) {
        m_GameSession->zoomCamera(1.0F);
        zoomTimer = 0.0F;
      } else if (Util::Input::IsKeyPressed(Util::Keycode::Q)) {
        m_GameSession->zoomCamera(-1.0F);
        zoomTimer = 0.0F;
      }
    }

    // 每幀順序：先更新邏輯狀態，再將結果渲染到螢幕
    m_GameSession->update(simDeltaTime, rawDeltaTime);
    if (!m_GameSession->isBaseAlive()) {
      // 遊戲結束：計入當局（含金幣殘值，血量為 0）
      m_Score += calcLevelScore(
          m_GameSession->getLevelNumber(),
          m_GameSession->getLoopCount(),
          0,  // 基地已被摧毀，血量貢獻為 0
          m_GameSession->getGold());
      LOG_INFO("Game Over. Total score={}, wave={}", m_Score, m_GameSession->getWave());
      m_Result = std::make_unique<Result>(m_GameSession->getWave(), m_Score);
      m_CurrentState = State::RESULT;
    } else if (m_GameSession->isLevelCompleted()) {
      // 關卡通關：計入本關完整分數（hp + gold + 過關獎勵）
      m_Score += calcLevelScore(
          m_GameSession->getLevelNumber(),
          m_GameSession->getLoopCount(),
          m_GameSession->getBaseHp(),
          m_GameSession->getGold());

      if (m_GameSession->getLevelNumber() == 5) {
        LOG_INFO("Level 5 completed. Final Score={}, transitioning to Result screen", m_Score);
        m_Result = std::make_unique<Result>(m_GameSession->getWave(), m_Score);
        m_CurrentState = State::RESULT;
      } else {
        const int nextLevelNumber = m_GameSession->getLevelNumber() + 1;
        LOG_INFO("Level completed. Score so far={}, loading level {}",
                 m_Score, nextLevelNumber);
        m_GameSession = std::make_unique<GameSession>(nextLevelNumber);
        m_GameSession->setCheatMode(m_CheatModeEnabled);
        m_SelectedTower = TowerId::Basic;
        m_GameSession->setSelectedTower(m_SelectedTower);
        m_GameSession->startSession();
      }
    }
    
    if (m_CurrentState == State::GAME) {
      m_GameSession->display();
    }
  }

  // 當按下 ESC 或視窗關閉時進入結束流程
  if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) || Util::Input::IfExit()) {
    m_CurrentState = State::END;
  }
}

// -------------------- 結束 --------------------
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void App::End() { LOG_TRACE("End"); }

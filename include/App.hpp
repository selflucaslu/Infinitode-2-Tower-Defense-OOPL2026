#ifndef APP_HPP
#define APP_HPP

#include "game/GameSession.hpp"
#include "pch.hpp" // IWYU pragma: export
#include "tower/TowerDef.hpp"
#include "ui/Home.hpp"

#include <memory>

class App {
public:
    // -------------------- App 主狀態 --------------------
    // 應用程式主流程狀態。
    enum class State {
        START,
        HOME,
        GAME,
        END,
    };

    // -------------------- 主迴圈介面 --------------------
    State GetCurrentState() const { return m_CurrentState; } // 取得目前流程狀態

    void Start(); // 初始化本局與資源

    void Update(); // 每幀更新

    void End(); // 結束流程（NOLINT: 之後會修改成員）

private:
    // -------------------- App 成員 --------------------
    State m_CurrentState = State::START; // App 流程狀態
    std::unique_ptr<Home> m_Home; // 首頁
    std::unique_ptr<GameSession> m_GameSession; // 本局執行期資料
    TowerId m_SelectedTower = TowerId::Basic; // 目前選中的塔種類
};

#endif

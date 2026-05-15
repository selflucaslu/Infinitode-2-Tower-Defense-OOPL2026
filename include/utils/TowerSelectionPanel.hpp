#pragma once

#include "tower/TowerDef.hpp"
#include "utils/AtlasLoader.hpp"

#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// -------------------- 選塔面板（右下角 HUD）--------------------
// 仿 Infinitode 2 風格，以格子顯示所有可建的塔種類，
// 選中的塔會有藍色高亮外框，並顯示塔費用。
class TowerSelectionPanel {
public:
    explicit TowerSelectionPanel(AtlasLoader& atlasLoader);

    // 每幀更新位置（需傳入視窗寬/高）
    void update(float windowWidth, float windowHeight);

    // 繪製面板
    void display();

    // 切換選中塔
    void setSelectedTower(TowerId id);
    TowerId getSelectedTower() const;

    // 根據滑鼠世界座標（螢幕座標）判斷點擊到哪個格子，回傳對應 TowerId
    // 若沒點到任何格子回傳 std::nullopt
    std::optional<TowerId> hitTest(float screenX, float screenY) const;

private:
    // 更新所有元件位置
    void layoutCells(float windowWidth, float windowHeight);

    // -------------------- 常數 --------------------
    static constexpr int   kColumns       = 2;          // 每行幾個格子
    static constexpr float kCellSize      = 72.0F;      // 格子大小（像素）
    static constexpr float kCellPadding   = 6.0F;       // 格子間距
    static constexpr float kPanelMarginX  = 16.0F;      // 距視窗右邊緣
    static constexpr float kPanelMarginY  = 16.0F;      // 距視窗下邊緣
    static constexpr float kIconScale     = 0.38F;      // 塔圖示縮放
    static constexpr float kBgZIndex      = 4.5F;       // 背景面板 Z
    static constexpr float kCellZIndex    = 4.6F;       // 格子背景 Z
    static constexpr float kIconZIndex    = 4.7F;       // 塔圖示 Z
    static constexpr float kHighlightZ    = 4.65F;      // 選中高亮框 Z
    static constexpr float kTextZIndex    = 4.8F;       // 費用文字 Z
    static constexpr const char* kFont    = "PTSD/assets/fonts/Inter.ttf";
    static constexpr int   kFontSize      = 16;

    // 單個格子的顯示資料
    struct Cell {
        TowerId towerId;
        glm::vec2 centerPos{0.0F, 0.0F}; // 螢幕中心座標（相對整個視窗中心）

        std::shared_ptr<Util::GameObject> cellBg;       // 格子暗色背景
        std::shared_ptr<Util::GameObject> highlight;    // 選中高亮框（橘/藍色邊框）
        std::shared_ptr<Util::GameObject> iconBase;     // 塔底座圖
        std::shared_ptr<Util::GameObject> iconWeapon;   // 塔砲管圖
        std::shared_ptr<Util::GameObject> costLabel;    // 費用文字物件
        std::shared_ptr<Util::Text>       costText;     // 費用文字資源
    };

    AtlasLoader& m_Atlas;
    TowerId      m_SelectedTower = TowerId::Basic;
    std::vector<Cell> m_Cells;
    Util::Renderer    m_Renderer;

    // 面板整體背景
    std::shared_ptr<Util::GameObject> m_PanelBg;
};

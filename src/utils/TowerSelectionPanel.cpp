#include "utils/TowerSelectionPanel.hpp"

#include "Core/Context.hpp"
#include "Util/Color.hpp"
#include "Util/Image.hpp"
#include "Util/Logger.hpp"

#include <cmath>
#include <optional>
#include <string>

// -------------------- 輔助：建立純色矩形 GameObject --------------------
// 由於 PTSD 的 Util::Image 只能載入檔案，我們透過一個 1×1 白色 PNG 搭配縮放來模擬填色背景。
// 如果沒有純白圖片，可以用任何正方形 sprite；這裡用 "icon-coins" 替代（稍後 tint 覆蓋）。
// ★ 實作細節：我們實際上用 Cell 的背景框直接縮放 icon sprite 並調整透明度，
//    高亮框則用相同方法但讓 tint 呈藍色系。

TowerSelectionPanel::TowerSelectionPanel(AtlasLoader& atlasLoader)
    : m_Atlas(atlasLoader) {

    // 預先建立每種塔的格子
    for (const TowerDef& def : kTowerDefs) {
        Cell cell;
        cell.towerId = def.id;

        // ---- 格子暗色背景 ----
        cell.cellBg = std::make_shared<Util::GameObject>();
        cell.cellBg->SetDrawable(m_Atlas.getImage("icon-tower"));
        cell.cellBg->SetZIndex(kCellZIndex);

        // ---- 選中高亮框 ----
        cell.highlight = std::make_shared<Util::GameObject>();
        cell.highlight->SetDrawable(m_Atlas.getImage("icon-tower-top"));
        cell.highlight->SetZIndex(kHighlightZ);
        cell.highlight->SetVisible(false); // 預設不顯示

        // ---- 塔底座圖示 ----
        cell.iconBase = std::make_shared<Util::GameObject>();
        cell.iconBase->SetDrawable(m_Atlas.getImage(std::string(def.spriteBase)));
        cell.iconBase->SetZIndex(kIconZIndex);

        // ---- 塔砲管圖示 ----
        cell.iconWeapon = std::make_shared<Util::GameObject>();
        cell.iconWeapon->SetDrawable(m_Atlas.getImage(std::string(def.spriteWeapon)));
        cell.iconWeapon->SetZIndex(kIconZIndex + 0.01F);

        // ---- 費用文字 ----
        cell.costText = std::make_shared<Util::Text>(
            kFont, kFontSize,
            std::to_string(def.buildCost),
            Util::Color::FromRGB(255, 220, 80) // 金黃色
        );
        cell.costLabel = std::make_shared<Util::GameObject>();
        cell.costLabel->SetDrawable(cell.costText);
        cell.costLabel->SetZIndex(kTextZIndex);

        // 加入 Renderer
        m_Renderer.AddChild(cell.cellBg);
        m_Renderer.AddChild(cell.highlight);
        m_Renderer.AddChild(cell.iconBase);
        m_Renderer.AddChild(cell.iconWeapon);
        m_Renderer.AddChild(cell.costLabel);

        m_Cells.push_back(std::move(cell));
    }

    // 預設選中第一個（Basic）
    setSelectedTower(TowerId::Basic);
}

// -------------------- 更新位置 --------------------
void TowerSelectionPanel::update(float windowWidth, float windowHeight) {
    layoutCells(windowWidth, windowHeight);
}

void TowerSelectionPanel::layoutCells(float windowWidth, float windowHeight) {
    const int rows = static_cast<int>(
        std::ceil(static_cast<float>(m_Cells.size()) / static_cast<float>(kColumns))
    );

    // 面板總尺寸
    const float panelW = kColumns * kCellSize + (kColumns - 1) * kCellPadding + kCellPadding * 2.0F;
    const float panelH = rows    * kCellSize + (rows    - 1) * kCellPadding + kCellPadding * 2.0F;

    // 面板右下角錨點（相對視窗中心的 OpenGL 座標）
    const float halfW  = windowWidth  * 0.5F;
    const float halfH  = windowHeight * 0.5F;
    const float panelLeft = halfW  - panelW - kPanelMarginX; // 面板左側 x
    const float panelTop  = -halfH + panelH + kPanelMarginY; // 面板頂部 y（往上為正）

    for (int idx = 0; idx < static_cast<int>(m_Cells.size()); ++idx) {
        Cell& cell = m_Cells[idx];
        const int col = idx % kColumns;
        const int row = idx / kColumns;

        // 格子中心（OpenGL 座標，y 往上為正）
        const float cx = panelLeft + kCellPadding + col * (kCellSize + kCellPadding) + kCellSize * 0.5F;
        const float cy = panelTop  - kCellPadding - row * (kCellSize + kCellPadding) - kCellSize * 0.5F;
        cell.centerPos = {cx, cy};

        // 格子背景縮放：用 icon-tower 當格子背景，縮放到 kCellSize
        const glm::vec2 bgSize = m_Atlas.getImage("icon-tower")->GetSize();
        const float bgScaleX = kCellSize / bgSize.x;
        const float bgScaleY = kCellSize / bgSize.y;
        cell.cellBg->m_Transform.translation = {cx, cy};
        cell.cellBg->m_Transform.scale       = {bgScaleX, bgScaleY};

        // 高亮框（比格子大一點點，用 icon-tower-top）
        const glm::vec2 hlSize = m_Atlas.getImage("icon-tower-top")->GetSize();
        const float hlScaleX = (kCellSize + 6.0F) / hlSize.x;
        const float hlScaleY = (kCellSize + 6.0F) / hlSize.y;
        cell.highlight->m_Transform.translation = {cx, cy};
        cell.highlight->m_Transform.scale       = {hlScaleX, hlScaleY};

        // 塔圖示：底座 + 砲管，疊在格子中心
        const float iconS = kCellSize * kIconScale;

        // 用各自塔的圖示大小重新計算
        const TowerDef& def = kTowerDefs[idx];
        const glm::vec2 myBaseSize   = m_Atlas.getImage(std::string(def.spriteBase))->GetSize();
        const glm::vec2 myWeaponSize = m_Atlas.getImage(std::string(def.spriteWeapon))->GetSize();

        const float myBaseScaleX   = iconS / myBaseSize.x;
        const float myBaseScaleY   = iconS / myBaseSize.y;
        const float myWeaponScaleX = iconS / myWeaponSize.x;
        const float myWeaponScaleY = iconS / myWeaponSize.y;

        cell.iconBase->m_Transform.translation   = {cx, cy + 4.0F};
        cell.iconBase->m_Transform.scale         = {myBaseScaleX, myBaseScaleY};
        cell.iconWeapon->m_Transform.translation = {cx, cy + 4.0F};
        cell.iconWeapon->m_Transform.scale       = {myWeaponScaleX, myWeaponScaleY};

        // 費用文字：置於格子底部
        const glm::vec2 textSize = cell.costText->GetSize();
        cell.costLabel->m_Transform.translation = {
            cx,
            cy - kCellSize * 0.5F + textSize.y * 0.6F + 4.0F
        };
    }
}

// -------------------- 選中塔 --------------------
void TowerSelectionPanel::setSelectedTower(TowerId id) {
    m_SelectedTower = id;
    for (Cell& cell : m_Cells) {
        const bool isSelected = (cell.towerId == id);
        cell.highlight->SetVisible(isSelected);
    }
}

TowerId TowerSelectionPanel::getSelectedTower() const {
    return m_SelectedTower;
}

// -------------------- 點擊偵測 --------------------
std::optional<TowerId> TowerSelectionPanel::hitTest(float screenX, float screenY) const {
    // screenX/Y 為相對視窗中心的 OpenGL 座標（左←負，右→正，上→正）
    const float halfCell = kCellSize * 0.5F;
    for (const Cell& cell : m_Cells) {
        if (screenX >= cell.centerPos.x - halfCell &&
            screenX <= cell.centerPos.x + halfCell &&
            screenY >= cell.centerPos.y - halfCell &&
            screenY <= cell.centerPos.y + halfCell)
        {
            return cell.towerId;
        }
    }
    return std::nullopt;
}

// -------------------- 繪製 --------------------
void TowerSelectionPanel::display() {
    const std::shared_ptr<Core::Context> context = Core::Context::GetInstance();
    const float windowWidth  = static_cast<float>(context->GetWindowWidth());
    const float windowHeight = static_cast<float>(context->GetWindowHeight());

    update(windowWidth, windowHeight);
    m_Renderer.Update();
}

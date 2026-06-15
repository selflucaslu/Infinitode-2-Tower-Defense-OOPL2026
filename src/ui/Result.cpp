#include "ui/Result.hpp"

#include "Core/Context.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

#include <SDL.h>

// -------------------- 建構子 --------------------
Result::Result(int wavesSurvived, int score) {
    m_Atlas.loadAtlas("assets/combined.atlas");
    createTextObjects(wavesSurvived, score);
    createButtons();
}

// -------------------- 每幀邏輯 --------------------
ResultAction Result::update() {
    const std::shared_ptr<Core::Context> context = Core::Context::GetInstance();
    layout(static_cast<float>(context->GetWindowWidth()),
           static_cast<float>(context->GetWindowHeight()));

    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    ResultAction action = ResultAction::None;

    for (Button& button : m_Buttons) {
        const bool hovered = isInsideButton(button, mousePos);
        button.highlight->SetVisible(hovered && !button.disabled);
        if (!button.disabled && hovered && Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            action = button.action;
        }
    }

    return action;
}

// -------------------- 繪製 --------------------
void Result::display() {
    glClearColor(24.0F / 255.0F, 24.0F / 255.0F, 24.0F / 255.0F, 1.0F);
    m_Renderer.Update();
}

// -------------------- 私有：建立文字物件 --------------------
void Result::createTextObjects(int wavesSurvived, int score) {
    // 標題「GAME OVER」
    m_TitleText = std::make_shared<Util::Text>(
        kFontPath, 52, "GAME OVER", Util::Color::FromRGB(255, 90, 90));
    m_TitleObject = std::make_shared<Util::GameObject>();
    m_TitleObject->SetDrawable(m_TitleText);
    m_TitleObject->SetZIndex(3.5F);
    m_Renderer.AddChild(m_TitleObject);

    // 分數行
    const std::string scoreStr = "Score: " + std::to_string(score);
    m_ScoreText = std::make_shared<Util::Text>(
        kFontPath, 32, scoreStr, Util::Color::FromRGB(255, 220, 80));
    m_ScoreObject = std::make_shared<Util::GameObject>();
    m_ScoreObject->SetDrawable(m_ScoreText);
    m_ScoreObject->SetZIndex(3.5F);
    m_Renderer.AddChild(m_ScoreObject);

    // 星級行（★ = U+2605, ☆ = U+2606）
    const int stars = calcStars(score);
    std::string starsStr;
    for (int i = 0; i < 3; ++i) {
        starsStr += (i < stars) ? "\xe2\x98\x85" : "\xe2\x98\x86"; // ★ or ☆ (UTF-8)
        if (i < 2) starsStr += " ";
    }

    // 附加閾值提示
    if (stars < 3) {
        const int nextThreshold = (stars == 0) ? k1StarThreshold
                                : (stars == 1) ? k2StarThreshold
                                               : k3StarThreshold;
        starsStr += "  (" + std::to_string(nextThreshold - score) + " pts to next star)";
    }

    m_StarsText = std::make_shared<Util::Text>(
        kFontPath, 28, starsStr, Util::Color::FromRGB(255, 200, 50));
    m_StarsObject = std::make_shared<Util::GameObject>();
    m_StarsObject->SetDrawable(m_StarsText);
    m_StarsObject->SetZIndex(3.5F);
    m_Renderer.AddChild(m_StarsObject);

    // 波次統計行
    const std::string statsStr = "Waves Survived: " + std::to_string(wavesSurvived);
    m_StatsText = std::make_shared<Util::Text>(
        kFontPath, 22, statsStr, Util::Color::FromRGB(180, 220, 255));
    m_StatsObject = std::make_shared<Util::GameObject>();
    m_StatsObject->SetDrawable(m_StatsText);
    m_StatsObject->SetZIndex(3.5F);
    m_Renderer.AddChild(m_StatsObject);
}

// -------------------- 私有：建立按鈕 --------------------
void Result::createButtons() {
    Button backButton;
    backButton.action  = ResultAction::BackToHome;
    backButton.size    = {220.0F, 60.0F};
    backButton.disabled = false;
    backButton.labelOffset = {0.0F, 0.0F};

    backButton.background = std::make_shared<Util::GameObject>();
    backButton.background->SetDrawable(m_Atlas.getImage("ui-money-screen-button-edge"));
    backButton.background->SetZIndex(2.8F);
    m_Renderer.AddChild(backButton.background);

    backButton.highlight = std::make_shared<Util::GameObject>();
    backButton.highlight->SetDrawable(m_Atlas.getImage("build-selection"));
    backButton.highlight->SetZIndex(2.9F);
    backButton.highlight->SetVisible(false);
    m_Renderer.AddChild(backButton.highlight);

    backButton.labelText = std::make_shared<Util::Text>(
        kFontPath, 20, "Return to Home", Util::Color::FromRGB(176, 236, 190));
    backButton.labelObject = std::make_shared<Util::GameObject>();
    backButton.labelObject->SetDrawable(backButton.labelText);
    backButton.labelObject->SetZIndex(3.0F);
    m_Renderer.AddChild(backButton.labelObject);

    m_Buttons.push_back(std::move(backButton));
}

// -------------------- 私有：版面配置 --------------------
void Result::layout(float /*windowWidth*/, float /*windowHeight*/) {
    // 垂直排列：標題 → 分數 → 星級 → 波次 → 按鈕
    m_TitleObject->m_Transform.translation  = {0.0F,  140.0F};
    m_ScoreObject->m_Transform.translation  = {0.0F,   76.0F};
    m_StarsObject->m_Transform.translation  = {0.0F,   32.0F};
    m_StatsObject->m_Transform.translation  = {0.0F,  -10.0F};

    for (Button& button : m_Buttons) {
        button.center = {0.0F, -80.0F};

        const glm::vec2 bgSize =
            m_Atlas.getImage("ui-money-screen-button-edge")->GetSize();
        button.background->m_Transform.translation = button.center;
        button.background->m_Transform.scale = {
            button.size.x / bgSize.x,
            button.size.y / bgSize.y,
        };

        const glm::vec2 hlSize =
            m_Atlas.getImage("build-selection")->GetSize();
        button.highlight->m_Transform.translation = button.center;
        button.highlight->m_Transform.scale = {
            (button.size.x + 8.0F) / hlSize.x,
            (button.size.y + 8.0F) / hlSize.y,
        };

        button.labelObject->m_Transform.translation =
            button.center + button.labelOffset;
    }
}

// -------------------- 私有：按鈕點擊判斷 --------------------
bool Result::isInsideButton(const Button& button, const glm::vec2& mousePos) const {
    const glm::vec2 half = button.size * 0.5F;
    return mousePos.x >= button.center.x - half.x &&
           mousePos.x <= button.center.x + half.x &&
           mousePos.y >= button.center.y - half.y &&
           mousePos.y <= button.center.y + half.y;
}

// -------------------- 靜態：星等計算 --------------------
int Result::calcStars(int score) {
    if (score >= k3StarThreshold) return 3;
    if (score >= k2StarThreshold) return 2;
    if (score >= k1StarThreshold) return 1;
    return 0;
}

#include "ui/Result.hpp"

#include "Core/Context.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

#include <SDL.h>

Result::Result(int wavesSurvived) {
    m_Atlas.loadAtlas("assets/combined.atlas");

    createTextObjects(wavesSurvived);
    createButtons();
}

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

void Result::display() {
    glClearColor(24.0F / 255.0F, 24.0F / 255.0F, 24.0F / 255.0F, 1.0F);
    m_Renderer.Update();
}

void Result::createTextObjects(int wavesSurvived) {
    m_TitleText = std::make_shared<Util::Text>(
        kFontPath, 48, "GAME OVER", Util::Color::FromRGB(255, 100, 100));
    m_TitleObject = std::make_shared<Util::GameObject>();
    m_TitleObject->SetDrawable(m_TitleText);
    m_TitleObject->SetZIndex(3.5F);
    m_Renderer.AddChild(m_TitleObject);

    const std::string statsString = "Waves Survived: " + std::to_string(wavesSurvived);
    m_StatsText = std::make_shared<Util::Text>(
        kFontPath, 24, statsString, Util::Color::FromRGB(238, 255, 255));
    m_StatsObject = std::make_shared<Util::GameObject>();
    m_StatsObject->SetDrawable(m_StatsText);
    m_StatsObject->SetZIndex(3.5F);
    m_Renderer.AddChild(m_StatsObject);
}

void Result::createButtons() {
    Button backButton;
    backButton.action = ResultAction::BackToHome;
    backButton.size = {200.0F, 60.0F};
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

void Result::layout(float windowWidth, float windowHeight) {
    (void)windowWidth; // unused
    (void)windowHeight; // unused

    m_TitleObject->m_Transform.translation = {0.0F, 100.0F};
    m_StatsObject->m_Transform.translation = {0.0F, 20.0F};

    for (Button& button : m_Buttons) {
        button.center = {0.0F, -80.0F};

        const glm::vec2 bgSize = button.background->GetDrawable()->GetSize();
        button.background->m_Transform.translation = button.center;
        button.background->m_Transform.scale = {
            button.size.x / bgSize.x,
            button.size.y / bgSize.y,
        };

        const glm::vec2 highlightSize = button.highlight->GetDrawable()->GetSize();
        button.highlight->m_Transform.translation = button.center;
        button.highlight->m_Transform.scale = {
            (button.size.x + 8.0F) / highlightSize.x,
            (button.size.y + 8.0F) / highlightSize.y,
        };

        button.labelObject->m_Transform.translation = button.center + button.labelOffset;
    }
}

bool Result::isInsideButton(const Button& button, const glm::vec2& mousePos) const {
    const glm::vec2 halfSize = button.size * 0.5F;
    return mousePos.x >= button.center.x - halfSize.x &&
           mousePos.x <= button.center.x + halfSize.x &&
           mousePos.y >= button.center.y - halfSize.y &&
           mousePos.y <= button.center.y + halfSize.y;
}

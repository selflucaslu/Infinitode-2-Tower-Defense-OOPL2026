#include "ui/Home.hpp"

#include "Core/Context.hpp"
#include "Util/Color.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

#include <SDL.h>

#include <filesystem>
#include <string>

Home::Home() {
    m_Atlas.loadAtlas("assets/combined.atlas");

    createTextObjects();
    createButtons();
    createAboutPopup();
}

HomeAction Home::update() {
    const std::shared_ptr<Core::Context> context = Core::Context::GetInstance();
    layout(static_cast<float>(context->GetWindowWidth()),
           static_cast<float>(context->GetWindowHeight()));

    const glm::vec2 mousePos = Util::Input::GetCursorPosition();
    HomeAction action = HomeAction::None;

    if (m_ShowAbout) {
        const glm::vec2 closeBtnCenter = {0.0F, -180.0F};
        const glm::vec2 closeBtnSize = {160.0F, 44.0F};
        const glm::vec2 halfSize = closeBtnSize * 0.5F;
        const bool closeHovered = mousePos.x >= closeBtnCenter.x - halfSize.x &&
                                  mousePos.x <= closeBtnCenter.x + halfSize.x &&
                                  mousePos.y >= closeBtnCenter.y - halfSize.y &&
                                  mousePos.y <= closeBtnCenter.y + halfSize.y;

        m_AboutCloseBtnHighlight->SetVisible(closeHovered);

        if (closeHovered && Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            m_ShowAbout = false;
            setAboutVisible(false);
        }

        if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE)) {
            m_ShowAbout = false;
            setAboutVisible(false);
        }

        for (Button& button : m_Buttons) {
            button.highlight->SetVisible(false);
        }

        return HomeAction::None;
    }

    for (Button& button : m_Buttons) {
        const bool hovered = isInsideButton(button, mousePos);
        button.highlight->SetVisible(hovered && !button.disabled);
        if (!button.disabled && hovered && Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            if (button.action == HomeAction::ShowAbout) {
                m_ShowAbout = true;
                setAboutVisible(true);
            } else {
                action = button.action;
            }
        }
    }

    return action;
}

void Home::display() {
    glClearColor(24.0F / 255.0F, 24.0F / 255.0F, 24.0F / 255.0F, 1.0F);
    m_Renderer.Update();
}

void Home::createTextObjects() {
    m_TitleText = std::make_shared<Util::Text>(
        kFontPath, 22, "Guest", Util::Color::FromRGB(238, 255, 255));
    m_TitleObject = std::make_shared<Util::GameObject>();
    m_TitleObject->SetDrawable(m_TitleText);
    m_TitleObject->SetZIndex(3.5F);
    m_Renderer.AddChild(m_TitleObject);

    m_SubtitleText = std::make_shared<Util::Text>(
        kFontPath, 13, "Tap here to sign in for cloud saves and leaderboards",
        Util::Color::FromRGB(135, 166, 166));
    m_SubtitleObject = std::make_shared<Util::GameObject>();
    m_SubtitleObject->SetDrawable(m_SubtitleText);
    m_SubtitleObject->SetZIndex(3.5F);
    m_Renderer.AddChild(m_SubtitleObject);

    m_UserIconObject = addIcon("icon-user", 3.5F, {0.38F, 0.38F});
    m_CoinIconObject = addIcon("game-ui-coin-icon", 3.5F, {0.42F, 0.42F});
    m_CoinTextObject = addText(18, "0", Util::Color::FromRGB(255, 208, 92), 3.5F);
    m_ResourceIconObject = addIcon("icon-cubes-stacked", 3.5F, {0.34F, 0.34F});
    m_ResourceTextObject = addText(18, "0", Util::Color::FromRGB(236, 255, 255), 3.5F);

}

void Home::createButtons() {
    const struct ButtonConfig {
        const char* label;
        const char* icon;
        const char* background;
        const char* fillName;
        Util::Color fillColor;
        HomeAction action;
        glm::vec2 size;
        bool disabled;
    } configs[] = {
        {"Music player", "icon-music-player", "ui-money-screen-button-small-top-edge",
         "home_music", Util::Color::FromRGB(26, 99, 130), HomeAction::None,
         {92.0F, 76.0F}, true},
        {"Settings", "icon-tools", "ui-money-screen-button-small-bottom-edge",
         "home_settings", Util::Color::FromRGB(23, 100, 132), HomeAction::None,
         {84.0F, 64.0F}, true},
        {"Handbook", "icon-book-closed", "ui-money-screen-button-small-bottom-edge",
         "home_handbook", Util::Color::FromRGB(22, 104, 136), HomeAction::None,
         {84.0F, 64.0F}, true},
        {"About", "icon-info", "ui-money-screen-button-small-bottom-edge",
         "home_about", Util::Color::FromRGB(23, 101, 134), HomeAction::ShowAbout,
         {84.0F, 64.0F}, false},
        {"Continue", "icon-double-triangle-right", "ui-money-screen-button-small-top-edge",
         "home_continue", Util::Color::FromRGB(63, 68, 74), HomeAction::None,
         {92.0F, 80.0F}, true},
        {"New game", "icon-joystick", "ui-money-screen-button-edge",
         "home_new_game", Util::Color::FromRGB(92, 133, 61), HomeAction::StartGame,
         {160.0F, 80.0F}, false},
        {"Statistics", "icon-statistics", "ui-money-screen-button-small-bottom-edge",
         "home_statistics", Util::Color::FromRGB(24, 108, 142), HomeAction::None,
         {84.0F, 64.0F}, true},
        {"Research", "icon-research", "ui-money-screen-button-small-bottom-edge",
         "home_research", Util::Color::FromRGB(24, 110, 143), HomeAction::None,
         {84.0F, 64.0F}, true},
        {"Custom maps", "icon-dat-paper", "ui-money-screen-button-small-bottom-edge",
         "home_custom_maps", Util::Color::FromRGB(104, 98, 92), HomeAction::None,
         {84.0F, 64.0F}, true},
    };

    for (const ButtonConfig& config : configs) {
        Button button;
        button.action = config.action;
        button.size = config.size;
        button.disabled = config.disabled;
        button.backgroundSpriteId = config.background;
        button.fillSize = {
            button.size.x - (button.size.x > 120.0F ? 18.0F : 14.0F),
            button.size.y - 14.0F,
        };
        button.iconScale = button.size.x > 120.0F ? 0.34F : 0.32F;
        button.iconOffset = button.size.x > 120.0F ? glm::vec2{-42.0F, 12.0F}
                                                   : glm::vec2{0.0F, 12.0F};
        button.labelOffset = button.size.x > 120.0F ? glm::vec2{28.0F, -11.0F}
                                                    : glm::vec2{0.0F, -16.0F};

        button.fillObject = addSolidPanel(
            config.fillName,
            glm::ivec2(static_cast<int>(button.fillSize.x), static_cast<int>(button.fillSize.y)),
            config.fillColor,
            2.7F
        );

        button.background = std::make_shared<Util::GameObject>();
        button.background->SetDrawable(m_Atlas.getImage(config.background));
        button.background->SetZIndex(2.8F);
        m_Renderer.AddChild(button.background);

        button.highlight = std::make_shared<Util::GameObject>();
        button.highlight->SetDrawable(m_Atlas.getImage("build-selection"));
        button.highlight->SetZIndex(2.9F);
        button.highlight->SetVisible(false);
        m_Renderer.AddChild(button.highlight);

        button.iconObject = addIcon(config.icon, 3.0F, {button.iconScale, button.iconScale});

        button.labelText = std::make_shared<Util::Text>(
            kFontPath, button.size.x > 120.0F ? 15 : 12, config.label,
            config.disabled ? Util::Color::FromRGB(154, 187, 190)
                            : Util::Color::FromRGB(176, 236, 190));
        button.labelObject = std::make_shared<Util::GameObject>();
        button.labelObject->SetDrawable(button.labelText);
        button.labelObject->SetZIndex(3.0F);
        m_Renderer.AddChild(button.labelObject);

        m_Buttons.push_back(std::move(button));
    }
}

void Home::layout(float windowWidth, float windowHeight) {
    const float halfW = windowWidth * 0.5F;
    const float halfH = windowHeight * 0.5F;
    constexpr float kSideMargin = 36.0F;
    constexpr float kBottomMargin = 28.0F;
    constexpr float kGap = 8.0F;

    m_TitleObject->m_Transform.translation = {
        -halfW + 76.0F + m_TitleText->GetSize().x * 0.5F,
        halfH - 52.0F,
    };
    m_SubtitleObject->m_Transform.translation = {
        -halfW + 76.0F + m_SubtitleText->GetSize().x * 0.5F,
        halfH - 76.0F,
    };
    m_UserIconObject->m_Transform.translation = {-halfW + 52.0F, halfH - 55.0F};

    m_CoinIconObject->m_Transform.translation = {halfW - 218.0F, halfH - 44.0F};
    m_CoinTextObject->m_Transform.translation = {halfW - 190.0F, halfH - 44.0F};
    m_ResourceIconObject->m_Transform.translation = {halfW - 118.0F, halfH - 44.0F};
    m_ResourceTextObject->m_Transform.translation = {halfW - 90.0F, halfH - 44.0F};

    for (std::size_t i = 0; i < m_Buttons.size(); ++i) {
        Button& button = m_Buttons[i];
        if (i == 0) {
            const float leftTopLeft = -halfW + kSideMargin;
            const float smallCenterY = -halfH + kBottomMargin + 32.0F;
            const float topCenterY = smallCenterY + 32.0F + kGap + 38.0F;
            button.center = {leftTopLeft + button.size.x * 0.5F, topCenterY};
        } else if (i >= 1 && i <= 3) {
            const float leftRowLeft = -halfW + kSideMargin;
            const float smallCenterY = -halfH + kBottomMargin + 32.0F;
            button.center = {
                leftRowLeft + 42.0F + static_cast<float>(i - 1) * (84.0F + kGap),
                smallCenterY
            };
        } else if (i == 4) {
            const float rightTopLeft = halfW - kSideMargin - (92.0F + kGap + 160.0F);
            const float rightSmallCenterY = -halfH + kBottomMargin + 32.0F;
            const float rightTopCenterY = rightSmallCenterY + 32.0F + kGap + 40.0F;
            button.center = {rightTopLeft + 46.0F, rightTopCenterY};
        } else if (i == 5) {
            const float rightTopLeft = halfW - kSideMargin - (92.0F + kGap + 160.0F);
            const float rightSmallCenterY = -halfH + kBottomMargin + 32.0F;
            const float rightTopCenterY = rightSmallCenterY + 32.0F + kGap + 40.0F;
            button.center = {rightTopLeft + 92.0F + kGap + 80.0F, rightTopCenterY};
        } else {
            const float rightRowLeft = halfW - kSideMargin - (84.0F * 3.0F + kGap * 2.0F);
            const float rightSmallCenterY = -halfH + kBottomMargin + 32.0F;
            button.center = {
                rightRowLeft + 42.0F + static_cast<float>(i - 6) * (84.0F + kGap),
                rightSmallCenterY
            };
        }

        const glm::vec2 bgSize = m_Atlas.getImage(button.backgroundSpriteId)->GetSize();
        button.fillObject->m_Transform.translation = button.center;
        button.fillObject->m_Transform.scale = {1.0F, 1.0F};
        button.background->m_Transform.translation = button.center;
        button.background->m_Transform.scale = {
            button.size.x / bgSize.x,
            button.size.y / bgSize.y,
        };

        const glm::vec2 highlightSize = m_Atlas.getImage("build-selection")->GetSize();
        button.highlight->m_Transform.translation = button.center;
        button.highlight->m_Transform.scale = {
            (button.size.x + 8.0F) / highlightSize.x,
            (button.size.y + 8.0F) / highlightSize.y,
        };

        button.iconObject->m_Transform.translation = button.center + button.iconOffset;
        button.labelObject->m_Transform.translation = button.center + button.labelOffset;
    }

    if (m_AboutDim) {
        m_AboutDim->m_Transform.translation = {0.0F, 0.0F};
        m_AboutDim->m_Transform.scale = {windowWidth / 2000.0F, windowHeight / 2000.0F};
    }
    if (m_AboutBorder) {
        m_AboutBorder->m_Transform.translation = {0.0F, 0.0F};
    }
    if (m_AboutDialog) {
        m_AboutDialog->m_Transform.translation = {0.0F, 0.0F};
    }
    if (m_AboutTitle) {
        m_AboutTitle->m_Transform.translation = {0.0F, 180.0F};
    }
    float startY = 110.0F;
    const float lineSpacing = 32.0F;
    for (std::size_t i = 0; i < m_AboutContentObjects.size(); ++i) {
        m_AboutContentObjects[i]->m_Transform.translation = {0.0F, startY - static_cast<float>(i) * lineSpacing};
    }
    if (m_AboutCloseBtn) {
        m_AboutCloseBtn->m_Transform.translation = {0.0F, -180.0F};
    }
    if (m_AboutCloseBtnHighlight) {
        m_AboutCloseBtnHighlight->m_Transform.translation = {0.0F, -180.0F};
        const glm::vec2 hlSize = m_Atlas.getImage("build-selection")->GetSize();
        m_AboutCloseBtnHighlight->m_Transform.scale = {
            (160.0F + 8.0F) / hlSize.x,
            (44.0F + 8.0F) / hlSize.y,
        };
    }
    if (m_AboutCloseBtnTextObj) {
        m_AboutCloseBtnTextObj->m_Transform.translation = {0.0F, -180.0F};
    }
}

bool Home::isInsideButton(const Button& button, const glm::vec2& mousePos) const {
    const glm::vec2 halfSize = button.size * 0.5F;
    return mousePos.x >= button.center.x - halfSize.x &&
           mousePos.x <= button.center.x + halfSize.x &&
           mousePos.y >= button.center.y - halfSize.y &&
           mousePos.y <= button.center.y + halfSize.y;
}

std::shared_ptr<Util::GameObject> Home::addIcon(
    const std::string& spriteId, float zIndex, glm::vec2 scale) {
    auto object = std::make_shared<Util::GameObject>();
    object->SetDrawable(m_Atlas.getImage(spriteId));
    object->SetZIndex(zIndex);
    object->m_Transform.scale = scale;
    m_Renderer.AddChild(object);
    m_StaticObjects.push_back(object);
    return object;
}

std::shared_ptr<Util::GameObject> Home::addText(
    int fontSize, const std::string& text, const Util::Color& color, float zIndex,
    std::shared_ptr<Util::Text>* textOut) {
    auto textDrawable = std::make_shared<Util::Text>(kFontPath, fontSize, text, color);
    auto object = std::make_shared<Util::GameObject>();
    object->SetDrawable(textDrawable);
    object->SetZIndex(zIndex);
    m_Renderer.AddChild(object);
    m_StaticTexts.push_back(textDrawable);
    m_StaticObjects.push_back(object);
    if (textOut != nullptr) {
        *textOut = textDrawable;
    }
    return object;
}

std::shared_ptr<Util::GameObject> Home::addSolidPanel(
    const std::string& cacheName, const glm::ivec2& pixelSize,
    const Util::Color& color, float zIndex) {
    namespace fs = std::filesystem;
    const fs::path cacheDir = fs::temp_directory_path() / "home_ui_cache";
    fs::create_directories(cacheDir);

    const fs::path imagePath = cacheDir / (cacheName + ".bmp");
    if (!fs::exists(imagePath)) {
        SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(
            0, pixelSize.x, pixelSize.y, 32, SDL_PIXELFORMAT_RGBA32
        );
        if (surface == nullptr) {
            throw std::runtime_error("SDL_CreateRGBSurfaceWithFormat failed");
        }

        SDL_FillRect(
            surface,
            nullptr,
            SDL_MapRGBA(
                surface->format,
                static_cast<Uint8>(color.r),
                static_cast<Uint8>(color.g),
                static_cast<Uint8>(color.b),
                static_cast<Uint8>(color.a)
            )
        );
        SDL_SaveBMP(surface, imagePath.string().c_str());
        SDL_FreeSurface(surface);
    }

    auto object = std::make_shared<Util::GameObject>();
    object->SetDrawable(std::make_shared<Util::Image>(imagePath.string()));
    object->SetZIndex(zIndex);
    m_Renderer.AddChild(object);
    return object;
}

void Home::createAboutPopup() {
    m_AboutDim = addSolidPanel("about_dim", {2000, 2000}, Util::Color::FromRGB(10, 10, 10, 180), 5.0F);
    m_AboutBorder = addSolidPanel("about_dialog_border", {654, 484}, Util::Color::FromRGB(255, 208, 92), 5.1F);
    m_AboutDialog = addSolidPanel("about_dialog", {650, 480}, Util::Color::FromRGB(30, 45, 54), 5.2F);
    m_AboutTitle = addText(24, "GAME CONTROLS & INSTRUCTIONS", Util::Color::FromRGB(255, 255, 255), 5.3F);

    const std::vector<std::string> instructions = {
        "WASD / Right-Mouse Drag   - Move Camera",
        "Mouse Scroll Wheel        - Zoom Camera (0.1x to 3.0x)",
        "Keys 1, 2, 3              - Select Tower Type (Basic, Sniper, Cannon)",
        "Left-Click on Platform    - Build Selected Tower",
        "Left-Click on UI Panel    - Choose Selected Tower Type",
        "Key X                    - Sell Tower at cursor (50% Gold refund)",
        "Key P                    - Show 'Pass Level' button (Testing)",
        "Key ESC                  - Close Menu / Exit Game"
    };

    for (const auto& line : instructions) {
        auto obj = addText(14, line, Util::Color::FromRGB(200, 220, 220), 5.3F);
        m_AboutContentObjects.push_back(obj);
    }

    m_AboutCloseBtn = addSolidPanel("about_close_btn", {160, 44}, Util::Color::FromRGB(92, 133, 61), 5.3F);

    m_AboutCloseBtnHighlight = std::make_shared<Util::GameObject>();
    m_AboutCloseBtnHighlight->SetDrawable(m_Atlas.getImage("build-selection"));
    m_AboutCloseBtnHighlight->SetZIndex(5.35F);
    m_AboutCloseBtnHighlight->SetVisible(false);
    m_Renderer.AddChild(m_AboutCloseBtnHighlight);

    m_AboutCloseBtnTextObj = addText(16, "Close", Util::Color::FromRGB(255, 255, 255), 5.4F, &m_AboutCloseBtnText);

    setAboutVisible(false);
}

void Home::setAboutVisible(bool visible) {
    m_AboutDim->SetVisible(visible);
    m_AboutBorder->SetVisible(visible);
    m_AboutDialog->SetVisible(visible);
    m_AboutTitle->SetVisible(visible);
    for (auto& obj : m_AboutContentObjects) {
        obj->SetVisible(visible);
    }
    m_AboutCloseBtn->SetVisible(visible);
    m_AboutCloseBtnTextObj->SetVisible(visible);
    if (!visible) {
        m_AboutCloseBtnHighlight->SetVisible(false);
    }
}

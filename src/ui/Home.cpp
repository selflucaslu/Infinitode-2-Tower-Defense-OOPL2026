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
    createHandbookPopup();
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

    if (m_ShowHandbook) {
        const glm::vec2 closeBtnCenter = {0.0F, -210.0F};
        const glm::vec2 closeBtnSize = {160.0F, 44.0F};
        const glm::vec2 halfClose = closeBtnSize * 0.5F;
        const bool closeHovered = mousePos.x >= closeBtnCenter.x - halfClose.x &&
                                  mousePos.x <= closeBtnCenter.x + halfClose.x &&
                                  mousePos.y >= closeBtnCenter.y - halfClose.y &&
                                  mousePos.y <= closeBtnCenter.y + halfClose.y;

        m_HandbookCloseBtnHighlight->SetVisible(closeHovered);

        if (closeHovered && Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
            m_ShowHandbook = false;
            setHandbookVisible(false);
        }

        if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE)) {
            m_ShowHandbook = false;
            setHandbookVisible(false);
        }

        for (int i = 0; i < 4; ++i) {
            auto& tab = m_TabButtons[i];
            const glm::vec2 halfTab = tab.size * 0.5F;
            const bool tabHovered = mousePos.x >= tab.center.x - halfTab.x &&
                                    mousePos.x <= tab.center.x + halfTab.x &&
                                    mousePos.y >= tab.center.y - halfTab.y &&
                                    mousePos.y <= tab.center.y + halfTab.y;

            if (tabHovered && Util::Input::IsKeyDown(Util::Keycode::MOUSE_LB)) {
                switchHandbookTab(i);
            }
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
            } else if (button.action == HomeAction::ShowHandbook) {
                m_ShowHandbook = true;
                setHandbookVisible(true);
                switchHandbookTab(0);
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
         "home_handbook", Util::Color::FromRGB(22, 104, 136), HomeAction::ShowHandbook,
         {84.0F, 64.0F}, false},
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
    float startY = 115.0F;
    const float lineSpacing = 34.0F;
    for (std::size_t i = 0; i < m_AboutContentObjects.size() / 2; ++i) {
        auto keyObj = m_AboutContentObjects[i * 2];
        auto descObj = m_AboutContentObjects[i * 2 + 1];

        auto keyText = m_AboutContentTexts[i * 2];
        auto descText = m_AboutContentTexts[i * 2 + 1];

        float y = startY - static_cast<float>(i) * lineSpacing;

        if (keyText) {
            float keyWidth = keyText->GetSize().x;
            keyObj->m_Transform.translation = {-270.0F + keyWidth * 0.5F, y};
        }
        if (descText) {
            float descWidth = descText->GetSize().x;
            descObj->m_Transform.translation = {-20.0F + descWidth * 0.5F, y};
        }
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

    // Handbook Popup Layout
    if (m_HandbookDim) {
        m_HandbookDim->m_Transform.translation = {0.0F, 0.0F};
        m_HandbookDim->m_Transform.scale = {windowWidth / 2000.0F, windowHeight / 2000.0F};
    }
    if (m_HandbookBorder) {
        m_HandbookBorder->m_Transform.translation = {0.0F, 0.0F};
    }
    if (m_HandbookDialog) {
        m_HandbookDialog->m_Transform.translation = {0.0F, 0.0F};
    }
    if (m_HandbookTitle) {
        m_HandbookTitle->m_Transform.translation = {0.0F, 215.0F};
    }

    // Tab buttons layout
    if (m_TabButtons.size() == 4) {
        const float tabXs[4] = {-228.0F, -76.0F, 76.0F, 228.0F};
        const float tabY = 160.0F;
        for (int i = 0; i < 4; ++i) {
            auto& tab = m_TabButtons[i];
            tab.center = {tabXs[i], tabY};
            tab.fillObject->m_Transform.translation = tab.center;
            tab.textObject->m_Transform.translation = tab.center;
        }
    }

    // Handbook page contents layout
    float hStartY = 95.0F;
    const float hLineSpacing = 36.0F;
    for (int pageIdx = 0; pageIdx < 4; ++pageIdx) {
        if (pageIdx >= static_cast<int>(m_HandbookPages.size())) break;
        auto& page = m_HandbookPages[pageIdx];
        for (std::size_t i = 0; i < page.keyObjects.size(); ++i) {
            auto keyObj = page.keyObjects[i];
            auto descObj = page.descObjects[i];
            auto keyText = page.keyTexts[i];
            auto descText = page.descTexts[i];

            float y = hStartY - static_cast<float>(i) * hLineSpacing;

            if (keyText) {
                float kw = keyText->GetSize().x;
                keyObj->m_Transform.translation = {-320.0F + kw * 0.5F, y};
            }
            if (descText) {
                float dw = descText->GetSize().x;
                descObj->m_Transform.translation = {-50.0F + dw * 0.5F, y};
            }
        }
    }

    if (m_HandbookCloseBtn) {
        m_HandbookCloseBtn->m_Transform.translation = {0.0F, -210.0F};
    }
    if (m_HandbookCloseBtnHighlight) {
        m_HandbookCloseBtnHighlight->m_Transform.translation = {0.0F, -210.0F};
        const glm::vec2 hlSize = m_Atlas.getImage("build-selection")->GetSize();
        m_HandbookCloseBtnHighlight->m_Transform.scale = {
            (160.0F + 8.0F) / hlSize.x,
            (44.0F + 8.0F) / hlSize.y,
        };
    }
    if (m_HandbookCloseBtnTextObj) {
        m_HandbookCloseBtnTextObj->m_Transform.translation = {0.0F, -210.0F};
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

    struct InstructionRow {
        std::string key;
        std::string desc;
    };
    const std::vector<InstructionRow> instructions = {
        {"WASD / Right-Mouse Drag",  "- Move Camera"},
        {"Mouse Scroll Wheel",       "- Zoom Camera (0.1x to 3.0x)"},
        {"Keys 1, 2, 3",             "- Select Tower (Basic / Sniper / Cannon)"},
        {"Left-Click on Platform",   "- Build Selected Tower"},
        {"Left-Click on UI Panel",   "- Choose Selected Tower Type"},
        {"Key X",                    "- Sell Tower at cursor (50% Refund)"},
        {"Key P",                    "- Show 'Pass Level' button (Testing)"},
        {"Key ESC",                  "- Close Menu / Exit Game"}
    };

    for (const auto& row : instructions) {
        std::shared_ptr<Util::Text> keyTextObj;
        std::shared_ptr<Util::Text> descTextObj;
        auto keyObj = addText(14, row.key, Util::Color::FromRGB(236, 255, 255), 5.3F, &keyTextObj);
        auto descObj = addText(14, row.desc, Util::Color::FromRGB(180, 220, 220), 5.3F, &descTextObj);
        m_AboutContentObjects.push_back(keyObj);
        m_AboutContentObjects.push_back(descObj);
        m_AboutContentTexts.push_back(keyTextObj);
        m_AboutContentTexts.push_back(descTextObj);
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

void Home::createHandbookPopup() {
    // 1. Pre-create active and inactive tab backgrounds to populate cache and then remove them from renderer
    auto activeBg = addSolidPanel("handbook_tab_active", {140, 36}, Util::Color::FromRGB(92, 133, 61), 5.8F);
    m_Renderer.RemoveChild(activeBg);
    auto inactiveBg = addSolidPanel("handbook_tab_inactive", {140, 36}, Util::Color::FromRGB(45, 60, 72), 5.8F);
    m_Renderer.RemoveChild(inactiveBg);

    // 2. Dimming background
    m_HandbookDim = addSolidPanel("handbook_dim", {2000, 2000}, Util::Color::FromRGB(10, 10, 10, 180), 5.5F);

    // 3. Dialog Border (Golden Border)
    m_HandbookBorder = addSolidPanel("handbook_border", {754, 524}, Util::Color::FromRGB(255, 208, 92), 5.6F);

    // 4. Dialog Box (Dark Blue-Grey)
    m_HandbookDialog = addSolidPanel("handbook_dialog", {750, 520}, Util::Color::FromRGB(30, 45, 54), 5.7F);

    // 5. Header Title
    m_HandbookTitle = addText(24, "GAME HANDBOOK", Util::Color::FromRGB(255, 255, 255), 5.8F);

    // 6. Tab Buttons setup
    const std::vector<std::string> tabNames = {"TOWERS", "ENEMIES", "LEVELS", "SCORING"};
    m_TabButtons.resize(4);
    for (int i = 0; i < 4; ++i) {
        TabButton tab;
        tab.size = {140.0F, 36.0F};
        tab.fillObject = addSolidPanel("handbook_tab_fill_" + std::to_string(i), {140, 36}, 
            (i == 0) ? Util::Color::FromRGB(92, 133, 61) : Util::Color::FromRGB(45, 60, 72), 5.8F);
        tab.textObject = addText(13, tabNames[i], 
            (i == 0) ? Util::Color::FromRGB(255, 255, 255) : Util::Color::FromRGB(170, 190, 200), 5.9F, &tab.textDrawable);
        m_TabButtons[i] = tab;
    }

    // 7. Handbook Pages content initialization
    m_HandbookPages.resize(4);

    struct KeyDescRow {
        std::string key;
        std::string desc;
    };

    // --- PAGE 0: TOWERS ---
    const std::vector<KeyDescRow> towersInfo = {
        {"Tower Type",                 "Cost  Range  Interval  Dmg  Bullet  Effect"},
        {"------------------------",   "-----------------------------------------------"},
        {"Basic Tower",                 " 39    7.0    0.35s     15   9.0     Single Target"},
        {"Sniper Tower",                " 79    14.0   1.20s     70   16.0    Single Target"},
        {"Cannon Tower",                " 99    6.0    1.50s     45   7.0     1.5-tile Splash"}
    };
    for (const auto& row : towersInfo) {
        std::shared_ptr<Util::Text> kTextObj;
        std::shared_ptr<Util::Text> dTextObj;
        bool isHeader = (row.key == "Tower Type" || row.key.find("---") != std::string::npos);
        Util::Color kColor = isHeader ? Util::Color::FromRGB(255, 208, 92) : Util::Color::FromRGB(236, 255, 255);
        Util::Color dColor = isHeader ? Util::Color::FromRGB(255, 208, 92) : Util::Color::FromRGB(180, 220, 220);

        auto kObj = addText(isHeader ? 14 : 13, row.key, kColor, 5.8F, &kTextObj);
        auto dObj = addText(isHeader ? 14 : 13, row.desc, dColor, 5.8F, &dTextObj);
        m_HandbookPages[0].keyObjects.push_back(kObj);
        m_HandbookPages[0].descObjects.push_back(dObj);
        m_HandbookPages[0].keyTexts.push_back(kTextObj);
        m_HandbookPages[0].descTexts.push_back(dTextObj);
    }

    // --- PAGE 1: ENEMIES ---
    const std::vector<KeyDescRow> enemiesInfo = {
        {"Enemy Types",                "Description & Core Characteristics"},
        {"------------------------",   "-----------------------------------------------"},
        {"Regular / Fast / Strong",    "- Ground: Standard traits, high speed, or high HP"},
        {"Heli / Jet",                  "- Aerial: Fly directly from Spawn to Goal (ignore roads)"},
        {"Armored / Healer",            "- Special: High physical armor / heals adjacent units"},
        {"Toxic / Icy",                 "- Elemental: Inflicts poison / slows down nearby towers"},
        {"Fighter / Light",             "- Tactical: Fighter/Light fast speed assault units"}
    };
    for (const auto& row : enemiesInfo) {
        std::shared_ptr<Util::Text> kTextObj;
        std::shared_ptr<Util::Text> dTextObj;
        bool isHeader = (row.key == "Enemy Types" || row.key.find("---") != std::string::npos);
        Util::Color kColor = isHeader ? Util::Color::FromRGB(255, 208, 92) : Util::Color::FromRGB(236, 255, 255);
        Util::Color dColor = isHeader ? Util::Color::FromRGB(255, 208, 92) : Util::Color::FromRGB(180, 220, 220);

        auto kObj = addText(isHeader ? 14 : 13, row.key, kColor, 5.8F, &kTextObj);
        auto dObj = addText(isHeader ? 14 : 13, row.desc, dColor, 5.8F, &dTextObj);
        m_HandbookPages[1].keyObjects.push_back(kObj);
        m_HandbookPages[1].descObjects.push_back(dObj);
        m_HandbookPages[1].keyTexts.push_back(kTextObj);
        m_HandbookPages[1].descTexts.push_back(dTextObj);
    }

    // --- PAGE 2: LEVELS ---
    const std::vector<KeyDescRow> levelsInfo = {
        {"Level Selection",             "Configurations (Map size, Starting HP & Gold, Waves)"},
        {"------------------------",   "-----------------------------------------------"},
        {"Level 1",                     "- Map: 7x3  | Life: 20  | Gold: 120 | Waves: 3 (Tutorial)"},
        {"Level 2",                     "- Map: 10x10 | Life: 20  | Gold: 160 | Waves: 5 (Aerial Intro)"},
        {"Level 3",                     "- Map: 29x30 | Life: 1   | Gold: 200 | Waves: 6 (High Density)"},
        {"Level 4",                     "- Map: 27x39 | Life: 100 | Gold: 200 | Waves: 5 (Mixed Specials)"},
        {"Level 5",                     "- Map: 51x51 | Life: 1   | Gold: 200 | Waves: 6 (Large City Grid)"}
    };
    for (const auto& row : levelsInfo) {
        std::shared_ptr<Util::Text> kTextObj;
        std::shared_ptr<Util::Text> dTextObj;
        bool isHeader = (row.key == "Level Selection" || row.key.find("---") != std::string::npos);
        Util::Color kColor = isHeader ? Util::Color::FromRGB(255, 208, 92) : Util::Color::FromRGB(236, 255, 255);
        Util::Color dColor = isHeader ? Util::Color::FromRGB(255, 208, 92) : Util::Color::FromRGB(180, 220, 220);

        auto kObj = addText(isHeader ? 14 : 13, row.key, kColor, 5.8F, &kTextObj);
        auto dObj = addText(isHeader ? 14 : 13, row.desc, dColor, 5.8F, &dTextObj);
        m_HandbookPages[2].keyObjects.push_back(kObj);
        m_HandbookPages[2].descObjects.push_back(dObj);
        m_HandbookPages[2].keyTexts.push_back(kTextObj);
        m_HandbookPages[2].descTexts.push_back(dTextObj);
    }

    // --- PAGE 3: SCORING ---
    const std::vector<KeyDescRow> scoringInfo = {
        {"Scoring Categories",          "Formulas & Target Star Thresholds"},
        {"------------------------",   "-----------------------------------------------"},
        {"Scoring Formula",             "Score = Remaining HP + floor(Gold / 1000) + Clear Bonus"},
        {"Clear Bonus L1 - L3",         "L1: 10 (+0/loop)  |  L2: 15 (+5/loop)  |  L3: 50 (+25/loop)"},
        {"Clear Bonus L4 - L5",         "L4: 25 (+10/loop) |  L5: 60 (+20/loop)"},
        {"Loop Scaling Effect",         "HP scaled by 1.2^loop | Speed scaled by 1.1^loop"},
        {"Star Thresholds",             "1 Star: 330 | 2 Stars: 660 | 3 Stars: 1000"}
    };
    for (const auto& row : scoringInfo) {
        std::shared_ptr<Util::Text> kTextObj;
        std::shared_ptr<Util::Text> dTextObj;
        bool isHeader = (row.key == "Scoring Categories" || row.key.find("---") != std::string::npos);
        Util::Color kColor = isHeader ? Util::Color::FromRGB(255, 208, 92) : Util::Color::FromRGB(236, 255, 255);
        Util::Color dColor = isHeader ? Util::Color::FromRGB(255, 208, 92) : Util::Color::FromRGB(180, 220, 220);

        auto kObj = addText(isHeader ? 14 : 13, row.key, kColor, 5.8F, &kTextObj);
        auto dObj = addText(isHeader ? 14 : 13, row.desc, dColor, 5.8F, &dTextObj);
        m_HandbookPages[3].keyObjects.push_back(kObj);
        m_HandbookPages[3].descObjects.push_back(dObj);
        m_HandbookPages[3].keyTexts.push_back(kTextObj);
        m_HandbookPages[3].descTexts.push_back(dTextObj);
    }

    // 8. Close button
    m_HandbookCloseBtn = addSolidPanel("handbook_close_btn", {160, 44}, Util::Color::FromRGB(92, 133, 61), 5.8F);

    m_HandbookCloseBtnHighlight = std::make_shared<Util::GameObject>();
    m_HandbookCloseBtnHighlight->SetDrawable(m_Atlas.getImage("build-selection"));
    m_HandbookCloseBtnHighlight->SetZIndex(5.85F);
    m_HandbookCloseBtnHighlight->SetVisible(false);
    m_Renderer.AddChild(m_HandbookCloseBtnHighlight);

    m_HandbookCloseBtnTextObj = addText(16, "Close", Util::Color::FromRGB(255, 255, 255), 5.9F, &m_HandbookCloseBtnText);

    setHandbookVisible(false);
}

void Home::setHandbookVisible(bool visible) {
    m_HandbookDim->SetVisible(visible);
    m_HandbookBorder->SetVisible(visible);
    m_HandbookDialog->SetVisible(visible);
    m_HandbookTitle->SetVisible(visible);
    for (auto& tab : m_TabButtons) {
        tab.fillObject->SetVisible(visible);
        tab.textObject->SetVisible(visible);
    }
    m_HandbookCloseBtn->SetVisible(visible);
    m_HandbookCloseBtnTextObj->SetVisible(visible);
    if (!visible) {
        m_HandbookCloseBtnHighlight->SetVisible(false);
    }

    for (int i = 0; i < 4; ++i) {
        bool pageVisible = visible && (i == m_ActiveHandbookTab);
        for (auto& obj : m_HandbookPages[i].keyObjects) {
            obj->SetVisible(pageVisible);
        }
        for (auto& obj : m_HandbookPages[i].descObjects) {
            obj->SetVisible(pageVisible);
        }
    }
}

void Home::switchHandbookTab(int tabIndex) {
    m_ActiveHandbookTab = tabIndex;
    namespace fs = std::filesystem;
    const fs::path cacheDir = fs::temp_directory_path() / "home_ui_cache";

    for (int i = 0; i < 4; ++i) {
        auto& tab = m_TabButtons[i];
        if (i == tabIndex) {
            tab.textDrawable->SetColor(Util::Color::FromRGB(255, 255, 255));
            fs::path activePath = cacheDir / "handbook_tab_active.bmp";
            tab.fillObject->SetDrawable(std::make_shared<Util::Image>(activePath.string()));
        } else {
            tab.textDrawable->SetColor(Util::Color::FromRGB(170, 190, 200));
            fs::path inactivePath = cacheDir / "handbook_tab_inactive.bmp";
            tab.fillObject->SetDrawable(std::make_shared<Util::Image>(inactivePath.string()));
        }
    }

    for (int i = 0; i < 4; ++i) {
        bool pageVisible = (i == tabIndex) && m_ShowHandbook;
        for (auto& obj : m_HandbookPages[i].keyObjects) {
            obj->SetVisible(pageVisible);
        }
        for (auto& obj : m_HandbookPages[i].descObjects) {
            obj->SetVisible(pageVisible);
        }
    }
}

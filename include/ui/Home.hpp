#pragma once

#include "Util/GameObject.hpp"
#include "Util/Color.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"
#include "utils/AtlasLoader.hpp"

#include <memory>
#include <string>
#include <vector>

enum class HomeAction {
    None,
    StartGame,
    Quit,
    ShowAbout,
    ShowHandbook,
};

class Home {
public:
    Home();

    HomeAction update();
    void display();

    bool isShowingAbout() const { return m_ShowAbout; }
    bool isShowingHandbook() const { return m_ShowHandbook; }

private:
    struct Button {
        HomeAction action = HomeAction::None;
        glm::vec2 center = {0.0F, 0.0F};
        glm::vec2 size = {0.0F, 0.0F};
        glm::vec2 labelOffset = {0.0F, 0.0F};
        glm::vec2 iconOffset = {0.0F, 0.0F};
        float iconScale = 0.5F;
        bool disabled = false;
        std::string backgroundSpriteId;
        glm::vec2 fillSize = {0.0F, 0.0F};
        std::shared_ptr<Util::GameObject> fillObject;
        std::shared_ptr<Util::GameObject> background;
        std::shared_ptr<Util::GameObject> highlight;
        std::shared_ptr<Util::GameObject> iconObject;
        std::shared_ptr<Util::GameObject> labelObject;
        std::shared_ptr<Util::Text> labelText;
    };

    struct HandbookPage {
        std::vector<std::shared_ptr<Util::GameObject>> keyObjects;
        std::vector<std::shared_ptr<Util::GameObject>> descObjects;
        std::vector<std::shared_ptr<Util::Text>> keyTexts;
        std::vector<std::shared_ptr<Util::Text>> descTexts;
    };

    struct TabButton {
        std::shared_ptr<Util::GameObject> fillObject;
        std::shared_ptr<Util::GameObject> textObject;
        std::shared_ptr<Util::Text> textDrawable;
        glm::vec2 center;
        glm::vec2 size;
    };

    void createTextObjects();
    void createButtons();
    void createAboutPopup();
    void setAboutVisible(bool visible);
    void createHandbookPopup();
    void setHandbookVisible(bool visible);
    void switchHandbookTab(int tabIndex);
    void layout(float windowWidth, float windowHeight);
    bool isInsideButton(const Button& button, const glm::vec2& mousePos) const;
    std::shared_ptr<Util::GameObject> addIcon(
        const std::string& spriteId, float zIndex, glm::vec2 scale = {1.0F, 1.0F});
    std::shared_ptr<Util::GameObject> addText(
        int fontSize, const std::string& text, const Util::Color& color, float zIndex,
        std::shared_ptr<Util::Text>* textOut = nullptr);
    std::shared_ptr<Util::GameObject> addSolidPanel(
        const std::string& cacheName, const glm::ivec2& pixelSize,
        const Util::Color& color, float zIndex);

private:
    static constexpr const char* kFontPath = "PTSD/assets/fonts/Inter.ttf";

    AtlasLoader m_Atlas;
    Util::Renderer m_Renderer;

    std::shared_ptr<Util::Text> m_TitleText;
    std::shared_ptr<Util::GameObject> m_TitleObject;
    std::shared_ptr<Util::Text> m_SubtitleText;
    std::shared_ptr<Util::GameObject> m_SubtitleObject;
    std::shared_ptr<Util::GameObject> m_UserIconObject;
    std::shared_ptr<Util::GameObject> m_CoinIconObject;
    std::shared_ptr<Util::GameObject> m_CoinTextObject;
    std::shared_ptr<Util::GameObject> m_ResourceIconObject;
    std::shared_ptr<Util::GameObject> m_ResourceTextObject;

    std::vector<std::shared_ptr<Util::Text>> m_StaticTexts;
    std::vector<std::shared_ptr<Util::GameObject>> m_StaticObjects;
    std::vector<Button> m_Buttons;

    // About Popup
    bool m_ShowAbout = false;
    std::shared_ptr<Util::GameObject> m_AboutDim;
    std::shared_ptr<Util::GameObject> m_AboutBorder;
    std::shared_ptr<Util::GameObject> m_AboutDialog;
    std::shared_ptr<Util::GameObject> m_AboutTitle;
    std::vector<std::shared_ptr<Util::GameObject>> m_AboutContentObjects;
    std::vector<std::shared_ptr<Util::Text>> m_AboutContentTexts;
    std::shared_ptr<Util::GameObject> m_AboutCloseBtn;
    std::shared_ptr<Util::GameObject> m_AboutCloseBtnHighlight;
    std::shared_ptr<Util::GameObject> m_AboutCloseBtnTextObj;
    std::shared_ptr<Util::Text> m_AboutCloseBtnText;

    // Handbook Popup
    bool m_ShowHandbook = false;
    int m_ActiveHandbookTab = 0;
    std::shared_ptr<Util::GameObject> m_HandbookDim;
    std::shared_ptr<Util::GameObject> m_HandbookBorder;
    std::shared_ptr<Util::GameObject> m_HandbookDialog;
    std::shared_ptr<Util::GameObject> m_HandbookTitle;
    std::vector<HandbookPage> m_HandbookPages;
    std::vector<TabButton> m_TabButtons;
    std::shared_ptr<Util::GameObject> m_HandbookCloseBtn;
    std::shared_ptr<Util::GameObject> m_HandbookCloseBtnHighlight;
    std::shared_ptr<Util::GameObject> m_HandbookCloseBtnTextObj;
    std::shared_ptr<Util::Text> m_HandbookCloseBtnText;
};



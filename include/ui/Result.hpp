#pragma once

#include "Util/Color.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"
#include "utils/AtlasLoader.hpp"

#include <memory>
#include <string>
#include <vector>

enum class ResultAction {
    None,
    BackToHome,
};

class Result {
public:
    explicit Result(int wavesSurvived);

    ResultAction update();
    void display();

private:
    struct Button {
        ResultAction action = ResultAction::None;
        glm::vec2 center = {0.0F, 0.0F};
        glm::vec2 size = {0.0F, 0.0F};
        glm::vec2 labelOffset = {0.0F, 0.0F};
        bool disabled = false;
        std::shared_ptr<Util::GameObject> background;
        std::shared_ptr<Util::GameObject> highlight;
        std::shared_ptr<Util::GameObject> labelObject;
        std::shared_ptr<Util::Text> labelText;
    };

    void createTextObjects(int wavesSurvived);
    void createButtons();
    void layout(float windowWidth, float windowHeight);
    bool isInsideButton(const Button& button, const glm::vec2& mousePos) const;

private:
    static constexpr const char* kFontPath = "PTSD/assets/fonts/Inter.ttf";

    AtlasLoader m_Atlas;
    Util::Renderer m_Renderer;

    std::shared_ptr<Util::Text> m_TitleText;
    std::shared_ptr<Util::GameObject> m_TitleObject;

    std::shared_ptr<Util::Text> m_StatsText;
    std::shared_ptr<Util::GameObject> m_StatsObject;

    std::vector<Button> m_Buttons;
};

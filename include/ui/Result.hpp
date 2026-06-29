#pragma once

#include "Util/Color.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"
#include "utils/AtlasLoader.hpp"

#include "Util/BGM.hpp"

#include <memory>
#include <string>
#include <vector>

enum class ResultAction {
    None,
    BackToHome,
};

class Result {
public:
    // wavesSurvived：存活波次；score：本局累計分數
    Result(int wavesSurvived, int score, int levelNumber);
    ~Result();

    ResultAction update();
    void display();

    // 星等判斷閾值（與需求一致）
    static constexpr int k1StarThreshold = 330;
    static constexpr int k2StarThreshold = 660;
    static constexpr int k3StarThreshold = 1000;

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

    void createTextObjects(int wavesSurvived, int score);
    void createButtons();
    void layout(float windowWidth, float windowHeight);
    bool isInsideButton(const Button& button, const glm::vec2& mousePos) const;

    static int calcStars(int score);

private:
    static constexpr const char* kFontPath = "PTSD/assets/fonts/Inter.ttf";

    AtlasLoader m_Atlas;
    Util::Renderer m_Renderer;

    // 標題
    std::shared_ptr<Util::Text> m_TitleText;
    std::shared_ptr<Util::GameObject> m_TitleObject;

    // 分數行
    std::shared_ptr<Util::Text> m_ScoreText;
    std::shared_ptr<Util::GameObject> m_ScoreObject;

    // 星級行
    std::shared_ptr<Util::Text> m_StarsText;
    std::shared_ptr<Util::GameObject> m_StarsObject;

    // 波次統計行
    std::shared_ptr<Util::Text> m_StatsText;
    std::shared_ptr<Util::GameObject> m_StatsObject;

    std::vector<Button> m_Buttons;

    std::unique_ptr<Util::BGM> m_ResultBgm;
};

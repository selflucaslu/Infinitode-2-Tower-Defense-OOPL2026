#include "utils/FpsOverlay.hpp"

#include "Core/Context.hpp"
#include "Util/Color.hpp"

#include <string>

FpsOverlay::FpsOverlay() {
    m_Text = std::make_shared<Util::Text>(
        kFontPath,
        kFontSize,
        "FPS: 0",
        Util::Color::FromRGB(255, 255, 255)
    );
    m_TextObject = std::make_shared<Util::GameObject>();
    m_TextObject->SetDrawable(m_Text);
    m_TextObject->SetZIndex(kZIndex); // 高於地圖(0)與敵人(1)(2)
}

void FpsOverlay::update(float rawDeltaTime) {
    if (!m_Text || rawDeltaTime <= 0.0F) {
        return;
    }

    // 降低更新頻率，避免每幀重建文字貼圖。
    m_UpdateTimer += rawDeltaTime;
    if (m_UpdateTimer < kRefreshInterval) {
        return;
    }

    const float fps = 1.0F / rawDeltaTime;
    m_Text->SetText("FPS: " + std::to_string(static_cast<int>(fps + 0.5F)));
    m_UpdateTimer = 0.0F;
}

void FpsOverlay::display() {
    if (!m_TextObject || !m_Text) {
        return;
    }

    const std::shared_ptr<Core::Context> context = Core::Context::GetInstance();
    const float halfWindowWidth = static_cast<float>(context->GetWindowWidth()) * 0.5F;
    const float halfWindowHeight = static_cast<float>(context->GetWindowHeight()) * 0.5F;
    const glm::vec2 textSize = m_Text->GetSize();

    m_TextObject->m_Transform.translation = {
        halfWindowWidth - textSize.x * 0.5F - kPadding,
        -halfWindowHeight + textSize.y * 0.5F + kPadding
    };
    m_TextObject->Draw();
}

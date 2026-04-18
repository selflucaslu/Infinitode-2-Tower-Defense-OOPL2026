#pragma once

#include "Util/GameObject.hpp"
#include "Util/Text.hpp"

#include <memory>

// -------------------- FPS 疊圖工具 --------------------
// 最小功能：
// 1) 吃 rawDeltaTime 計算 FPS（不受 simDeltaTime clamp 影響）
// 2) 以白色文字顯示在右上角
class FpsOverlay {
public:
    FpsOverlay();

    // 用原始幀時間更新數字（秒）。
    void update(float rawDeltaTime);

    // 每幀繪製到右上角。
    void display();

private:
    std::shared_ptr<Util::Text> m_Text;
    std::shared_ptr<Util::GameObject> m_TextObject;
    float m_UpdateTimer = 0.0F;
};

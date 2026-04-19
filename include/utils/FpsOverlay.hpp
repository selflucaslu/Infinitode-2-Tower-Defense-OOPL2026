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
    // Change this:
    static constexpr const char* kFontPath = "assets/fonts/Inter.ttf";
    static constexpr int kFontSize = 24; // 字體大小
    static constexpr float kZIndex = 3.0F; // 疊圖的 Z 索引，確保在其他 UI 元素之上
    static constexpr float kRefreshInterval = 0.2F; // 更新 FPS 顯示的時間間隔（秒）
    static constexpr float kPadding = 16.0F; // 與螢幕邊緣的距離（像素）

    std::shared_ptr<Util::Text> m_Text;
    std::shared_ptr<Util::GameObject> m_TextObject;
    float m_UpdateTimer = 0.0F;
};

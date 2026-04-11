#pragma once

#include "Util/Image.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>  // 用於存儲圖集名稱和對應的Bounds


struct Bounds {
    int x; // 左上角 x
    int y; // 左上角 y
    int w; // 寬度
    int h; // 高度
};

class AtlasLoader {
public:
    void loadAtlas(std::string_view filePath); // 載入 atlas 索引資料
    Bounds getBounds(std::string_view name) const; // 查詢指定名稱的 bounds
    std::shared_ptr<Util::Image> getImage(std::string_view name); // 取得（含快取）裁切後圖片
private:
    std::string atlasImagePath; // atlas 主圖路徑
    std::unordered_map<std::string, Bounds> atlasMap; // 名稱 -> 範圍資料
    std::unordered_map<std::string, std::shared_ptr<Util::Image>> m_Cache; // 名稱 -> 已載入圖片快取
};

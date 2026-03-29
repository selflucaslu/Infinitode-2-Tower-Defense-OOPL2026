#pragma once

#include "Util/Image.hpp"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>  // 用於存儲圖集名稱和對應的Bounds


struct Bounds {
    int x;
    int y;
    int w;
    int h;
};

class AtlasLoader {
public:
    void loadAtlas(const std::string_view filePath);
    Bounds getBounds(const std::string_view name) const;
    std::shared_ptr<Util::Image> getImage(const std::string_view name);
private:
    std::string atlasImagePath;
    std::unordered_map<std::string, Bounds> atlasMap;
    std::unordered_map<std::string, std::shared_ptr<Util::Image>> m_Cache;
};

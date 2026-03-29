#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "map/GridMap.hpp"

#include <memory>

namespace {
std::unique_ptr<GridMap> g_Map;
}

void App::Start() {
    LOG_TRACE("Start");
    g_Map = std::make_unique<GridMap>("assets/maps/map_01.csv");

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    if (g_Map) {
        g_Map->displayMap();
    }
    
    /*
     * Do not touch the code below as they serve the purpose for
     * closing the window.
     */
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() { // NOLINT(this method will mutate members in the future)
    LOG_TRACE("End");
}

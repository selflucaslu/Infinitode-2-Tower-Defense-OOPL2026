#include "App.hpp"

#include "Core/Context.hpp"
#include "Util/Logger.hpp"

#include <cstdlib>
#include <exception>

int main(int, char**) {
    try {
        // 建立全域執行環境與 App 主流程。
        auto context = Core::Context::GetInstance();
        App app;

        // 最小主迴圈：依 App 狀態切換 Start/Update/End。
        while (!context->GetExit()) {
            switch (app.GetCurrentState()) {
                case App::State::START:
                    app.Start();
                    break;

                case App::State::UPDATE:
                    app.Update();
                    break;

                case App::State::END:
                    app.End();
                    context->SetExit(true);
                    break;
            }
            context->Update();
        }
        return EXIT_SUCCESS;
    } catch (const std::exception& e) {
        LOG_ERROR("Fatal error: {}", e.what());
        return EXIT_FAILURE;
    } catch (...) {
        LOG_ERROR("Fatal error: unknown exception");
        return EXIT_FAILURE;
    }
}

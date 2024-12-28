#pragma once

#include <Application.hpp>

#include "State.hpp"

namespace lodeb {
    class AppLayer: public Scaffold::IRenderUI, public Scaffold::IUpdate {
    public:
        AppLayer();

        void OnUpdate(float dt) override;
        void OnRenderUI(float dt) override;

    private:
        State state;

        void WindowTargetSettings();
        void WindowCommandBar();
        void WindowSourceView();
        void WindowProcessOutput();
        void WindowDebug();
    };
}

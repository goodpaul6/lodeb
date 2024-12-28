#pragma once

#include <Application.hpp>

#include "State.hpp"

namespace lodeb {
    class AppLayer: public Scaffold::IRenderUI {
    public:
        void OnRenderUI(float dt) override;

    private:
        State state;
    };
}

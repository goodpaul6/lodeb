#pragma once

#include <Application.hpp>

namespace lodeb {
    class AppLayer: public Scaffold::IRenderUI {
    public:
        void OnRenderUI(float dt) override;
    };
}

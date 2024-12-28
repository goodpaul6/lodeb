#include "AppLayer.hpp"

#include <imgui.h>

namespace lodeb {
    void AppLayer::OnRenderUI(float) {
        ImGui::Begin("Hello");
        ImGui::TextUnformatted("Lodeb is here");
        ImGui::End();
    }
}

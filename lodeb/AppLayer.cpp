#include "AppLayer.hpp"

#include <imgui.h>
#include <lldb/API/LLDB.h>

namespace lodeb {
    void AppLayer::OnRenderUI(float) {
        ImGui::Begin("Hello");
        ImGui::TextUnformatted("Lodeb is here");
        ImGui::End();
    }
}

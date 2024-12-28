#include "AppLayer.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <tinyfiledialogs.h>
#include <lldb/API/LLDB.h>

namespace lodeb {
    void AppLayer::OnUpdate(float) {
        state.ProcessEvents();
    }

    void AppLayer::OnRenderUI(float) {
        WindowTargetSettings();
    }

    void AppLayer::WindowTargetSettings() {
        ImGui::Begin("Target Settings");

        ImGui::InputText("Exe Path", &state.target_settings.exe_path);
        ImGui::SameLine();

        if(ImGui::Button("Browse##exe_path")) {
            const char* path = tinyfd_openFileDialog(
                "Path to Executable", 
                nullptr, 
                0, 
                nullptr, 
                nullptr, 
                0
            );

            if(path) {
                state.target_settings.exe_path = path;
            }
        }

        ImGui::InputText("Working Dir", &state.target_settings.working_dir);
        ImGui::SameLine();

        if(ImGui::Button("Browse##working_dir")) {
            const char* path = tinyfd_selectFolderDialog("Working Directory", nullptr);

            if(path) {
                state.target_settings.working_dir = path;
            }
        }

        if(ImGui::Button("Load Target")) {
            state.events.push_back(RequestLoadTargetEvent{});
        }

        ImGui::End();
    }
}

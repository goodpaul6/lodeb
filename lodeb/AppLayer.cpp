#include "AppLayer.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <tinyfiledialogs.h>
#include <lldb/API/LLDB.h>
#include <sstream>

#include <stdio.h>

#include "ParseCommand.hpp"
#include "Log.hpp"

namespace {
    using namespace lodeb;

    const char* COMMAND_BAR_POPUP_NAME = "Command Bar";

    std::optional<FileLoc> SymLoc(lldb::SBSymbol& sym) {
        auto addr = sym.GetStartAddress();
        
        if(!addr.IsValid()) {
            return std::nullopt;
        }

        auto le = addr.GetLineEntry();
        auto fs = le.GetFileSpec();

        char buf[1024];

        fs.GetPath(buf, sizeof(buf));

        return FileLoc{
            .path = buf,
            .line = static_cast<int>(le.GetLine()),
        };
    }

    bool ReadEntireFileInto(const char* path, std::string& into) {
        FILE* f = fopen(path, "rb");

        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);

        into.resize(size);

        rewind(f);
        size_t n = fread(into.data(), 1, size, f);

        fclose(f);

        return n == size;
    }
}

namespace lodeb {
    using namespace Scaffold;
    
    void AppLayer::OnUpdate(float) {
        state.ProcessEvents();
    }

    void AppLayer::OnRenderUI(float) {
        WindowTargetSettings();
        WindowCommandBar();
        WindowSourceView();
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
            state.events.push_back(LoadTargetEvent{});
        }

        ImGui::End();
    }

    void AppLayer::WindowCommandBar() {
        auto handle_parsed_command = [&](ParsedCommand& parsed) {
            auto& target_state = state.target_state;

            if(!target_state) {
                ImGui::Text("No target loaded");

                return;
            }

            auto* sym_search = std::get_if<LookForSymbolCommand>(&parsed);
            if(!sym_search) {
                return;
            }

            // TODO(Apaar): Handle file search
            ImGui::BeginChild("##symbols", {300, 300}, ImGuiChildFlags_NavFlattened);
            
            for(auto mod_i = 0u; mod_i < target_state->target.GetNumModules(); mod_i++) {
                auto mod = target_state->target.GetModuleAtIndex(mod_i);

                ImGui::PushID(mod_i);

                for(auto sym_i = 0u; sym_i < mod.GetNumSymbols(); ++sym_i) {
                    auto sym = mod.GetSymbolAtIndex(sym_i);

                    std::string_view name = sym.GetName();

                    if(name.find(sym_search->text) == std::string_view::npos) {
                        continue;
                    }

                    ImGui::PushID(sym_i);

                    if(ImGui::Selectable(sym.GetName())) {
                        auto loc = SymLoc(sym);

                        if(loc) {
                            ViewSourceEvent event{*loc};

                            LogInfo("Pushing ViewSourceEvent {}", *loc);

                            state.events.push_back(std::move(event));
                        }

                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::PopID();
                }

                ImGui::PopID();
            }

            ImGui::EndChild();
        };

        auto& input = Application::GetInput();

        if(input.GetKeyState(KeyCode::P) == KeyState::Pressed &&
           (input.IsKeyDown(KeyCode::LeftControl) || 
            input.IsKeyDown(KeyCode::RightControl) ||
            input.IsKeyDown(KeyCode::LeftSuper) ||
            input.IsKeyDown(KeyCode::RightSuper))) {
            ImGui::OpenPopup(COMMAND_BAR_POPUP_NAME);
        }

        if(ImGui::BeginPopup(COMMAND_BAR_POPUP_NAME)) {
            if(!state.cmd_bar_state) {
                state.cmd_bar_state.emplace();
            }

            if(!state.cmd_bar_state->focused_text) {
                ImGui::SetKeyboardFocusHere();
                state.cmd_bar_state->focused_text = true;
            }

            ImGui::InputText("##command_bar_text", &state.cmd_bar_state->text);

            auto parsed = ParseCommand(state.cmd_bar_state->text);
            handle_parsed_command(parsed);

            ImGui::EndPopup();
        } else {
            state.cmd_bar_state.reset();
        }
    }

    void AppLayer::WindowSourceView() {
        auto& source_view_state = state.source_view_state;

        if(!source_view_state) {
            return;
        }

        if(!source_view_state->path.empty() &&
            source_view_state->text.empty()) {
            ReadEntireFileInto(source_view_state->path.c_str(), source_view_state->text);            
            LogInfo("Loaded file {}", source_view_state->path);
        }

        ImGui::Begin("Source View");

        ImGui::Text("Path: %s", source_view_state->path.c_str());

        ImGui::BeginChild("##text", {-1, -1});

        std::string line_buf;

        std::istringstream ss{source_view_state->text};

        for(std::string line; std::getline(ss, line);) {
            ImGui::TextUnformatted(line.c_str());
        }

        ImGui::EndChild();

        ImGui::End();
    }
}

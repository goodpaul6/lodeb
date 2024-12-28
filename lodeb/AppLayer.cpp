#include "AppLayer.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>
#include <tinyfiledialogs.h>
#include <lldb/API/LLDB.h>
#include <sstream>

#include <stdio.h>

#include "ParseCommand.hpp"
#include "Log.hpp"
#include "LLDBUtil.hpp"

namespace {
    using namespace lodeb;

    const char* COMMAND_BAR_POPUP_NAME = "Command Bar";
    const char* STATE_PATH = "lodeb.txt";

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

    AppLayer::AppLayer() {
        state.Load(STATE_PATH);
    }
    
    void AppLayer::OnUpdate(float) {
        state.Update();
        state.Store(STATE_PATH);
    }

    void AppLayer::OnRenderUI(float) {
        WindowTargetSettings();
        WindowCommandBar();
        WindowSourceView();
        WindowProcessOutput();
        WindowDebug();
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
            ImGui::BeginChild("##symbols", {400, 300}, ImGuiChildFlags_NavFlattened);
            
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

        auto* viewport = ImGui::GetMainViewport();

        ImGui::SetNextWindowPos(
            {viewport->Pos.x + viewport->Size.x * 0.5f,
             viewport->Pos.y + 200},
            ImGuiCond_Appearing,
            {0.5f, 0.5f}
        );

        if(!ImGui::BeginPopup(COMMAND_BAR_POPUP_NAME)) {
            state.cmd_bar_state.reset();
            return;
        }

        if(!state.cmd_bar_state) {
            state.cmd_bar_state.emplace();
        }

        if(!state.cmd_bar_state->focused_text) {
            ImGui::SetKeyboardFocusHere();
            state.cmd_bar_state->focused_text = true;
        }

        ImGui::SetNextItemWidth(400.0f);

        ImGui::InputText("##command_bar_text", &state.cmd_bar_state->text);

        auto parsed = ParseCommand(state.cmd_bar_state->text);
        handle_parsed_command(parsed);

        ImGui::EndPopup();
    }

    void AppLayer::WindowSourceView() {
        auto& source_view_state = state.source_view_state;

        if(!source_view_state) {
            return;
        }

        auto get_cur_frame_loc = [&]() -> std::optional<FileLoc> {
            if(!state.target_state ||
               !state.target_state->process_state &&
               state.target_state->process_state->process.GetState() != lldb::eStateStopped) {
                return std::nullopt;
            }

            auto cur_thread = state.target_state->process_state->process.GetSelectedThread();
            if(!cur_thread.IsValid()) {
                return std::nullopt;
            }

            auto cur_frame = cur_thread.GetSelectedFrame();

            return FrameLoc(cur_frame);
        };

        auto cur_frame_loc = get_cur_frame_loc();

        if(!source_view_state->path.empty() &&
            source_view_state->text.empty()) {
            ReadEntireFileInto(source_view_state->path.c_str(), source_view_state->text);            
            LogInfo("Loaded file {}", source_view_state->path);
        }

        ImGui::Begin("Source View");

        ImGui::TextUnformatted(source_view_state->path.c_str());

        ImGui::BeginChild("##text", {-1, -1}, ImGuiChildFlags_Border, ImGuiWindowFlags_NoNav);;

        std::string line_buf;

        std::istringstream ss{source_view_state->text};

        FileLoc loc = {
            .path = source_view_state->path,
            .line = 0,
        };

        for(std::string line; (loc.line += 1), std::getline(ss, line);) {
            ImGui::PushID(loc.line);

            line_buf.clear();
            std::format_to(std::back_inserter(line_buf), "{:5} {}", loc.line, line);

            if(ImGui::InvisibleButton("##gutter", {16, 16})) {
                state.events.push_back(ToggleBreakpointEvent{loc});
            }

            ImGui::SameLine();

            bool has_bp = state.target_state && state.target_state->loc_to_breakpoint.contains(loc);

            if(has_bp) {
                auto* draw_list = ImGui::GetWindowDrawList();
                auto pos = ImGui::GetItemRectMin();
                draw_list->AddCircleFilled(
                    {pos.x + 10, pos.y + 10},
                    5,
                    ImGui::GetColorU32(ImVec4{1.0, 0.0, 0.0, 1.0})
                );

                ImGui::SameLine();
            }

            bool highlight = cur_frame_loc == loc;

            if(highlight) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImVec4{0.25, 0.5, 1.0, 1.0}));
            }


            ImGui::TextUnformatted(line_buf.c_str());

            if(highlight) {
                ImGui::PopStyleColor();
            }

            if(source_view_state->scroll_to_line == loc.line) {
                ImGui::SetScrollHereY();
            }

            ImGui::PopID();
        }

        source_view_state->scroll_to_line.reset();

        ImGui::EndChild();

        ImGui::End();
    }

    void AppLayer::WindowProcessOutput() {
        ImGui::Begin("Process Output");

        ImGui::BeginChild("##text", {-1, -1}, ImGuiChildFlags_Border);

        ImGui::TextUnformatted(state.process_output.c_str());

        ImGui::EndChild();
        ImGui::End();
    }

    void AppLayer::WindowDebug() {
        ImGui::Begin("Debug");

        if(!state.target_state) {
            ImGui::Text("No target loaded");

            ImGui::End();
            return;
        }

        if(!state.target_state->process_state) {
            if(ImGui::Button("Start")) {
                state.events.push_back(StartProcessEvent{});
            }
            
            ImGui::End();
            return;
        }

        auto& ps = state.target_state->process_state;

        if(ps->process.GetState() != lldb::eStateStopped) {
            ImGui::End();
            return;
        }

        if(ImGui::Button("Kill")) {
            state.events.push_back(ChangeDebugStateEvent{ChangeDebugStateEvent::Kill});
        }

        if(ImGui::Button("Step In")) {
            state.events.push_back(ChangeDebugStateEvent{ChangeDebugStateEvent::StepIn});
        }

        ImGui::SameLine();

        if(ImGui::Button("Step Over")) {
            state.events.push_back(ChangeDebugStateEvent{ChangeDebugStateEvent::StepOver});
        }

        ImGui::SameLine();

        if(ImGui::Button("Continue")) {
            state.events.push_back(ChangeDebugStateEvent{ChangeDebugStateEvent::Continue});
        }

        ImGui::End();
    }
}

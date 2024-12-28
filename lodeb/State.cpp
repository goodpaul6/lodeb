#include "State.hpp"

#include <fstream>

#include <lldb/API/LLDB.h>

#include "Log.hpp"

namespace lodeb {
    State::State() : debugger{lldb::SBDebugger::Create()} {
        debugger.SetAsync(true);
    }

    State::~State() {
        lldb::SBDebugger::Destroy(debugger);
    }

    void State::Load(const char* path) {
        std::ifstream file{path};
        
        std::string buf;
        int version = 1;

        for(;;) {
            if(!(file >> buf)) {
                break;
            }

            if(buf == "version") {
                file >> version;
            }

            if(buf == "target_settings.exe_path") {
                file >> std::ws >> std::quoted(target_settings.exe_path);
            }

            if(buf == "target_settings.working_dir") {
                file >> std::ws >> std::quoted(target_settings.working_dir);
            }
        }
    }

    void State::Store(const char* path) {
        std::ofstream file{path};

        file << "version 1\n";

        file << "target_settings.exe_path " << std::quoted(target_settings.exe_path) << '\n';
        file << "target_settings.working_dir " << std::quoted(target_settings.working_dir) << '\n';
    }

    void State::ProcessEvents() {
        for(const auto& event : events) {
            if(auto* load_target = std::get_if<LoadTargetEvent>(&event)) {
                target_state = {
                    .target = debugger.CreateTarget(target_settings.exe_path.c_str()),
                };
                
                LogInfo("Created target {}", target_settings.exe_path);
            } else if(auto* view_source = std::get_if<ViewSourceEvent>(&event)) {
                if(source_view_state && source_view_state->path == view_source->loc.path) {
                    source_view_state->scroll_to_line = view_source->loc.line;
                } else {
                    source_view_state = {
                        .path = view_source->loc.path,
                        .scroll_to_line = view_source->loc.line,
                    };
                }
            }
        }

        events.clear();
    }
}


#include "State.hpp"

#include <lldb/API/LLDB.h>

#include "Log.hpp"

namespace lodeb {
    State::State() : debugger{lldb::SBDebugger::Create()} {
        debugger.SetAsync(true);
    }

    State::~State() {
        lldb::SBDebugger::Destroy(debugger);
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


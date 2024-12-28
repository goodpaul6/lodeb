#include "State.hpp"

#include <lldb/API/LLDB.h>

#include "Log.hpp"

namespace lodeb {
    State::State() : debugger{lldb::SBDebugger::Create()} {
    }

    State::~State() {
        lldb::SBDebugger::Destroy(debugger);
    }

    void State::ProcessEvents() {
        for(const auto& event : events) {
            if(auto* load_target = std::get_if<RequestLoadTargetEvent>(&event)) {
                target_state = {
                    .target = debugger.CreateTarget(target_settings.exe_path.c_str()),
                };
                
                LogInfo("Created target {}", target_settings.exe_path);
            }
        }

        events.clear();
    }
}


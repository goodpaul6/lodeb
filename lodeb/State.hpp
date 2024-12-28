#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <lldb/API/LLDB.h>

namespace lodeb {
    struct TargetSettings {
        std::string exe_path;
        std::string working_dir;
    };

    struct TargetState {
        lldb::SBTarget target;
    };

    struct RequestLoadTargetEvent {};

    using StateEvent = std::variant<RequestLoadTargetEvent>;

    struct State {
        lldb::SBDebugger debugger;

        std::vector<StateEvent> events;

        TargetSettings target_settings;

        std::optional<TargetState> target_state;

        State();
        ~State();    

        void ProcessEvents();
    };

    void ProcessEvents(State& state);
}

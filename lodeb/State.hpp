#pragma once

#include <optional>
#include <string>

#include <lldb/API/LLDB.h>

namespace lodeb {
    struct TargetSettings {
        std::string exe_path;
        std::string working_dir;
    };

    struct TargetState {
        lldb::SBTarget target;
    };

    struct State {
        lldb::SBDebugger debugger;

        TargetSettings target_settings;

        std::optional<TargetState> target_state;

        State();
        ~State();
    };
}

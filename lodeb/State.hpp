#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <format>

#include <lldb/API/LLDB.h>

namespace lodeb {
    struct FileLoc {
        std::string path;
        int line = 0;
    };

    struct TargetSettings {
        std::string exe_path;
        std::string working_dir;
    };

    struct TargetState {
        lldb::SBTarget target;
    };

    struct CommandBarState {
        std::string text;
        bool focused_text = false;
    };

    struct SourceViewState {
        std::string path;
        std::string text;

        // Only stays valid for one frame
        std::optional<int> scroll_to_line;
    };
    
    struct ProcessState {
        lldb::SBProcess process;
    };

    struct LoadTargetEvent {};
    struct ViewSourceEvent {
        FileLoc loc;
    };
    struct StartProcessEvent {};

    using StateEvent = std::variant<
        LoadTargetEvent, 
        ViewSourceEvent,
        StartProcessEvent
    >;

    struct State {
        lldb::SBDebugger debugger;

        std::vector<StateEvent> events;

        TargetSettings target_settings;

        std::optional<CommandBarState> cmd_bar_state;
        std::optional<TargetState> target_state;
        std::optional<ProcessState> process_state;

        State();
        ~State();    

        void ProcessEvents();
    };
}

template <>
struct std::formatter<lodeb::FileLoc> {
    template <typename ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) const {
        return ctx.begin();
    }

    template <typename FmtContext>
    FmtContext::iterator format(lodeb::FileLoc loc, FmtContext& ctx) const {
        return std::format_to(ctx.out(), "{}:{}", loc.path, loc.line);
    }
};

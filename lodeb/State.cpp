#include "State.hpp"

#include <fstream>
#include <cassert>

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

        auto init = [&]<typename T>(std::optional<T>& opt) -> std::optional<T>& {
            if(!opt) {
                opt.emplace();
            }

            return opt;
        };

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

            if(buf == "source_view_state.path") {
                file >> std::ws >> std::quoted(init(source_view_state)->path);
            }
        }

        if(!target_settings.exe_path.empty()) {
            // Request a target load right away because 99% chance we're just testing
            // the same binary
            events.push_back(LoadTargetEvent{});
        }
    }

    void State::Store(const char* path) {
        std::ofstream file{path};

        file << "version 1\n";

        file << "target_settings.exe_path " << std::quoted(target_settings.exe_path) << '\n';
        file << "target_settings.working_dir " << std::quoted(target_settings.working_dir) << '\n';

        if(source_view_state) {
            file << "source_view_state.path " << std::quoted(source_view_state->path) << '\n';
        }
    }

    void State::Update() {
        auto handle_process = [&] {
            if(!target_state || !target_state->process_state) {
                return;
            }

            auto& ps = *target_state->process_state;

            // Listen for process events
            lldb::SBEvent process_event;

            while(ps.listener.GetNextEvent(process_event)) {
                auto state = lldb::SBProcess::GetStateFromEvent(process_event);

                if(state == lldb::eStateStopped) {
                    LogInfo("Process stopped");

                    // Select the thread which was stopped due to breakpoint
                    for(auto i = 0u; i < ps.process.GetNumThreads(); ++i) {
                        auto t = ps.process.GetThreadAtIndex(i);

                        if(t.GetStopReason() == lldb::eStopReasonBreakpoint) {
                            ps.process.SetSelectedThread(t);
                            break;
                        }
                    }
                } else if(state == lldb::eStateExited || state == lldb::eStateDetached || state == lldb::eStateUnloaded) {
                    LogInfo("Process exited");

                    // All done, stop
                    target_state->process_state.reset();
                    return;
                }
            }

            
            // Poll the output
            char buf[2048] = {0};

            size_t n = ps.process.GetSTDOUT(buf, sizeof(buf) - 1);
            if(n >= 0) {
                buf[n] = '\0';
            }

            process_output += buf;

            n = ps.process.GetSTDERR(buf, sizeof(buf) - 1);
            if(n >= 0) {
                buf[n] = '\0';
            }

            process_output += buf;
        };

        handle_process();

        for(const auto& event : events) {
            if(auto* load_target = std::get_if<LoadTargetEvent>(&event)) {
                target_state = {
                    .target = debugger.CreateTarget(target_settings.exe_path.c_str()),
                };
                
                LogInfo("Created target {}", target_settings.exe_path);
            } else if(auto* view_source = std::get_if<ViewSourceEvent>(&event)) {
                LogDebug("View source event {}", view_source->loc);

                if(source_view_state && source_view_state->path == view_source->loc.path) {
                    source_view_state->scroll_to_line = view_source->loc.line;
                } else {
                    source_view_state = {
                        // We shant access the loc path again since we're clearing out these events
                        .path = std::move(view_source->loc.path),
                        .scroll_to_line = view_source->loc.line,
                    };
                }
            } else if(auto* start_process = std::get_if<StartProcessEvent>(&event)) {
                assert(target_state);
                process_output.clear();

                auto listener = debugger.GetListener();

                lldb::SBLaunchInfo li{nullptr};

                li.SetWorkingDirectory(target_settings.working_dir.c_str());
                li.SetListener(listener);

                lldb::SBError err;

                auto process = target_state->target.Launch(li, err);
                
                if(!process.IsValid() || err.Fail()) {
                    LogError("Failed to start process: {}", err.GetCString());
                    continue;
                }

                LogInfo("Started process {}", target_settings.exe_path);

                target_state->process_state = {
                    .listener = std::move(listener),
                    .process = std::move(process),
                };
            } else if(auto* toggle_bp = std::get_if<ToggleBreakpointEvent>(&event)) {
                assert(target_state);

                auto found = target_state->loc_to_breakpoint.find(toggle_bp->loc);

                if(found == target_state->loc_to_breakpoint.end()) {
                    LogInfo("Adding breakpoint to {}", toggle_bp->loc);

                    target_state->loc_to_breakpoint[toggle_bp->loc] = target_state->target.BreakpointCreateByLocation(
                        toggle_bp->loc.path.c_str(),
                        toggle_bp->loc.line
                    );
                    continue;
                }

                target_state->target.BreakpointDelete(found->second.GetID());

                LogInfo("Removing breakpoint from {}", toggle_bp->loc);

                target_state->loc_to_breakpoint.erase(found);
            }
        }

        events.clear();
    }
}


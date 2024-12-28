#include "State.hpp"

#include <lldb/API/LLDB.h>

namespace lodeb {
    State::State() : debugger{lldb::SBDebugger::Create()} {
    }

    State::~State() {
        lldb::SBDebugger::Destroy(debugger);
    }
}


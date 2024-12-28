#pragma once

#include "State.hpp"

namespace lodeb {
    std::optional<FileLoc> LineEntryLoc(const lldb::SBLineEntry& le);
    std::optional<FileLoc> AddrLoc(lldb::SBAddress& addr);
    std::optional<FileLoc> SymLoc(lldb::SBSymbol& sym);    
    std::optional<FileLoc> FrameLoc(lldb::SBFrame& frame);
    std::optional<FileLoc> ThreadLoc(lldb::SBThread& thread);
}

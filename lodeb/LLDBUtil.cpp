#include "LLDBUtil.hpp"

namespace lodeb {
    std::optional<FileLoc> LineEntryLoc(const lldb::SBLineEntry& le) {
        if(!le.IsValid()) {
            return std::nullopt;
        }

        auto fs = le.GetFileSpec();

        char buf[1024];

        fs.GetPath(buf, sizeof(buf));

        return FileLoc{
            .path = buf,
            .line = static_cast<int>(le.GetLine()),
        };
    }

    std::optional<FileLoc> AddrLoc(lldb::SBAddress& addr) {
        if(!addr.IsValid()) {
            return std::nullopt;
        }

        auto le = addr.GetLineEntry();
        return LineEntryLoc(le);
    }

    std::optional<FileLoc> SymLoc(lldb::SBSymbol& sym) {
        auto addr = sym.GetStartAddress();
        return AddrLoc(addr);
    }
    
    std::optional<FileLoc> FrameLoc(lldb::SBFrame& frame) {
        if(!frame.IsValid()) {
            return std::nullopt;
        }

        auto le = frame.GetLineEntry();
        return LineEntryLoc(le);
    }

    std::optional<FileLoc> ThreadLoc(lldb::SBThread& thread) {
        if(!thread.IsValid()) {
            return std::nullopt;
        }

        auto frame = thread.GetSelectedFrame();

        return FrameLoc(frame);
    }
}

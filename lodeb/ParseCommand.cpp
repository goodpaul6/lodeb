#include "ParseCommand.hpp"

namespace lodeb {
    ParsedCommand ParseCommand(std::string_view text) {
        if(text.starts_with("@")) {
            text.remove_prefix(1);
            return LookForSymbolCommand{text};
        }

        return LookForFileCommand{text};
    }
}

#pragma once

#include <variant>
#include <string_view>

namespace lodeb {
    struct LookForFileCommand {
        std::string_view text;
    };

    struct LookForSymbolCommand {
        std::string_view text;
    };

    using ParsedCommand = std::variant<LookForFileCommand, LookForSymbolCommand>;

    ParsedCommand ParseCommand(std::string_view text);
}

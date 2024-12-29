#pragma once

namespace lodeb {
    struct FileLoc {
        std::string path;
        int line = 0;

        bool operator==(const FileLoc&) const = default;
    };
}

template <>
struct std::hash<lodeb::FileLoc> {
    std::size_t operator()(const lodeb::FileLoc& loc) const noexcept {
        size_t h1 = std::hash<std::string>{}(loc.path);
        size_t h2 = std::hash<int>{}(loc.line);

        return h1 ^ (h2 << 1);
    }
};


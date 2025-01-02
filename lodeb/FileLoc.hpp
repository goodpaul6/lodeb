#pragma once

#include <string>

namespace lodeb {
    template <typename Str>
    struct GenericFileLoc {
        Str path;
        int line = 0; 

        bool operator==(const GenericFileLoc&) const = default;
    };
    
    using FileLoc = GenericFileLoc<std::string>;
    using FileLocView = GenericFileLoc<std::string_view>;

    inline FileLoc to_owned(const FileLocView& view) {
        return {std::string{view.path}, view.line};
    }
}

template <typename Str>
struct std::hash<lodeb::GenericFileLoc<Str>> {
    std::size_t operator()(const lodeb::GenericFileLoc<Str>& loc) const noexcept {
        size_t h1 = std::hash<Str>{}(loc.path);
        size_t h2 = std::hash<int>{}(loc.line);

        return h1 ^ (h2 << 1);
    }
};


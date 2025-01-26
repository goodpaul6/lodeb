#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <cctype>

#include <lldb/API/LLDB.h>

#include "FileLoc.hpp"

namespace lodeb {
    // A symbol->loc cache for our interactive search which needs to
    // be blazingly fast (tm).
    class SymbolLocCache {
        // Every single symbol name is just put into here one after
        // another. This lets us use the fast string.find method to
        // look for symbols that match.
        //
        // For every match, we do binary search in the locs below
        // to grab the corresponding loc which contains the located index.
        std::string names;

        // To make symbol search case-insensitive (assumes ASCII)
        std::string lowercase_names;
        
        // We pool paths here so we can use FileLocView above
        std::unordered_set<std::string> file_paths;

        struct NameRangeLoc {
            size_t start = 0;
            uint32_t len = 0;

            FileLocView loc;
        };
        
        std::vector<NameRangeLoc> locs;
    public:
        struct Match {
            std::string_view name;
            const FileLocView* loc = nullptr;
        };

        void Load(lldb::SBTarget& target);

        size_t SymbolCount() const { return locs.size(); }

        template <typename Fn>
        void ForEachMatch(std::string_view search, Fn&& fn, size_t limit) {
            if(locs.empty()) {
                return;
            }

            std::string search_buf{search};
            
            for(auto& c : search_buf) {
                c = std::tolower(c);
            }

            const auto loc_to_match = [&](NameRangeLoc& loc) {
                return Match{
                    .name = std::string_view{names}.substr(loc.start, loc.len),
                    .loc = &loc.loc,
                };
            };

            size_t count = 0;
            
            if(search_buf.empty()) {
                for(auto& loc: locs) {
                    count += 1;
                    if(count > limit) {
                        break;
                    }

                    fn(loc_to_match(loc));
                }

                return;
            }

            for(size_t pos = 0; (pos = lowercase_names.find(search_buf, pos)), pos != std::string::npos; pos += 1) { 
                count += 1;
                if(count > limit) {
                    break;
                }

                size_t lo = 0;
                auto hi = locs.size() - 1;

                while(lo <= hi) {
                    auto mid = (lo + hi) / 2;
                    auto& loc = locs[mid];

                    if(loc.start <= pos && loc.start + loc.len > pos) {
                        fn(loc_to_match(loc));

                        // Skip over this symbol in the names (-1 because pos += 1 in the for loop 'next')
                        pos = loc.start + loc.len - 1;
                        break;
                    } else if(loc.start > pos) {
                        hi = mid - 1;
                    } else {
                        lo = mid + 1;
                    }
                }
            }
        }    
    };
}

#pragma once

#include <vector>
#include <string>

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

        struct NameRangeLoc {
            size_t start = 0;
            uint32_t len = 0;

            FileLoc loc;
        };
        
        std::vector<NameRangeLoc> locs;
    public:
        struct Match {
            std::string_view name;
            const FileLoc* loc = nullptr;
        };

        void Load(lldb::SBTarget& target);

        template <typename Fn>
        void ForEachMatch(std::string_view search, Fn&& fn) {
            if(locs.empty()) {
                return;
            }

            const auto loc_to_match = [&](NameRangeLoc& loc) {
                return Match{
                    .name = std::string_view{names}.substr(loc.start, loc.len),
                    .loc = &loc.loc,
                };
            };
            
            if(search.empty()) {
                for(auto& loc: locs) {
                    fn(loc_to_match(loc));
                }

                return;
            }

            for(size_t pos = 0; (pos = names.find(search, pos)), pos != std::string::npos; pos += 1) { 
                size_t lo = 0;
                auto hi = locs.size() - 1;

                while(lo <= hi) {
                    auto mid = (lo + hi) / 2;
                    auto& loc = locs[mid];

                    if(loc.start <= pos && loc.start + loc.len > pos) {
                        fn(loc_to_match(loc));
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
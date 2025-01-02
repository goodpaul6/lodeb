#include "SymbolLocCache.hpp"

#include "LLDBUtil.hpp"

namespace lodeb {
    void SymbolLocCache::Load(lldb::SBTarget& target) {
        for(auto mod_i = 0u; mod_i < target.GetNumModules(); ++mod_i) {
            auto mod = target.GetModuleAtIndex(mod_i);

            for(auto sym_i = 0u; sym_i < mod.GetNumSymbols(); ++sym_i) {
                auto sym = mod.GetSymbolAtIndex(sym_i);
                
                // TODO(Apaar): Cache different symbol types
                if(sym.GetType() != lldb::eSymbolTypeCode) {
                    continue;
                }

                // TODO(Apaar): This allocates a new string on every loop. It's fine because
                // we're gonna be moving it into `file_paths` anyways but `AddrLoc` just uses
                // a stack-allocated buffer so it might be wise to just avoid calling the util
                // functions if we ever see this being a bottleneck.
                auto loc = SymLoc(sym);

                if(!loc) {
                    continue;
                }

                std::string_view name = sym.GetName();

                auto start = names.size();
                names.append(name);

                auto [inserted_iter, inserted] = file_paths.insert(std::move(loc->path));

                locs.push_back(NameRangeLoc{
                    .start = start,
                    .len = static_cast<uint32_t>(name.size()),
                    .loc = FileLocView{
                        .path = *inserted_iter,
                        .line = loc->line,
                    },
                });
            }
        }
    }
}

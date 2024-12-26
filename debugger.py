from typing import Optional
from collections import defaultdict
from dataclasses import dataclass, field
from concurrent.futures import Future, ThreadPoolExecutor

import lldb

@dataclass
class ExeParams:
    exe_path: str = ""
    working_dir: str = ""

@dataclass
class FileInfo:
    symbols: list[lldb.SBSymbol] = field(default_factory=list)

@dataclass
class TargetMetadata:
    path_to_files: dict[str, FileInfo] = field(default_factory=lambda: defaultdict(lambda: FileInfo()))

def create() -> lldb.SBDebugger:
    dbg = lldb.SBDebugger.Create()

    # No docs on how to do async mode so...
    dbg.SetAsync(False)

    return dbg


def create_target(dbg: lldb.SBDebugger, ep: ExeParams) -> Optional[lldb.SBTarget]:
    return dbg.CreateTargetWithFileAndArch(ep.exe_path, lldb.LLDB_ARCH_DEFAULT)


def get_target_metadata_wait(target: lldb.SBTarget) -> TargetMetadata:
    md = TargetMetadata()

    for mod in target.modules:
        for sym in mod:
            if sym.GetStartAddress().GetFileAddress() == 0:
                continue

            path = sym.GetStartAddress().GetLineEntry().GetFileSpec().fullpath

            if path:
                md.path_to_files[path].symbols.append(sym)

    return md

def create_breakpoint_by_file_line(target: lldb.SBTarget, fname: str, line: int) -> lldb.SBBreakpoint:
    return target.BreakpointCreateByLocation(fname, line)

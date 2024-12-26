from typing import Optional
from concurrent.futures import Future
from dataclasses import dataclass, field

import lldb
import debugger

@dataclass
class Loc:
    path: str
    line: int


@dataclass
class SourceFile:
    path: str
    text: str

    # If this is set, then the source view will scroll this
    # line into view. It will then set this to None.
    scroll_to_line: Optional[int]


@dataclass
class State:
    dbg: lldb.SBDebugger

    exe_params: debugger.ExeParams
    should_load: bool = False

    target: Optional[lldb.SBTarget] = None
    target_metadata: Optional[Future[debugger.TargetMetadata]] = None

    sym_search_text: str = ""
    
    # Clicking this will show this loc in the source view
    loc_to_open: Optional[Loc] = None

    source_file: Optional[SourceFile] = None

def create() -> State:
    return State(
        dbg = debugger.create(),
        exe_params = debugger.ExeParams()
    )

def sym_loc(sym: lldb.SBSymbol) -> Loc:
    addr = sym.addr
    sym_path = addr.line_entry.file.fullpath
    sym_line = sym.addr.line_entry.line

    return Loc(path=sym_path, line=sym_line)

from typing import Optional
from concurrent.futures import Future
from dataclasses import dataclass, field

import lldb
import debugger

@dataclass
class LocatedSym:
    fi: debugger.FileInfo
    path: str
    sym: lldb.SBSymbol


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

    # When the user clicks on a symbol anywhere in the UI
    # we can open that in the source window
    sym_to_open: Optional[LocatedSym] = None

    sym_search_text: str = ""
    
    source_file: Optional[SourceFile] = None

def create() -> State:
    return State(
        dbg = debugger.create(),
        exe_params = debugger.ExeParams()
    )

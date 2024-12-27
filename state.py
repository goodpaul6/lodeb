from typing import Optional
from concurrent.futures import Future
from dataclasses import dataclass, field, asdict
import threading
import json

import lldb
import debugger

@dataclass(frozen=True)
class Loc:
    path: str
    line: int


@dataclass
class SourceFile:
    path: str
    text: str

    # If this is set, then the source view will scroll this
    # line into view. It will then set this to None.
    scroll_to_line: Optional[int] = None

@dataclass
class ProcessState:
    process: lldb.SBProcess

    should_kill: bool = False

    should_step_in: bool = False
    should_step_over: bool = False
    should_continue: bool = False

    run_to_loc: Optional[Loc] = None

    highlight_loc: Optional[Loc] = None


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

    # When set, we attempt to add a breakpoint at this location
    loc_to_toggle_breakpoint: Optional[Loc] = None

    loc_to_breakpoint: dict[Loc, lldb.SBBreakpoint] = field(default_factory=dict)

    should_start: bool = False

    process: Optional[ProcessState] = None
    output: Optional[debugger.ProcessOutput] = None


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

def load(st: State, path: str):
    with open(path) as f:
        data = json.load(f)

        st.exe_params = debugger.ExeParams(**data.get('exe_params', {}))
        source_path = data.get('source_file_path')

        if source_path:
            with open(source_path) as f:
                st.source_file = SourceFile(
                    path=source_path,
                    text=f.read(),
                )


def store(st: State, path: str):
    with open(path, 'w') as f:
        json.dump({
            'exe_params': asdict(st.exe_params),
            'source_file_path': st.source_file and st.source_file.path,
        }, f)

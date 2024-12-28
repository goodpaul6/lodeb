import os
from typing import Optional
from collections import defaultdict
from dataclasses import dataclass, field
from concurrent.futures import Future, ThreadPoolExecutor
from threading import Thread, Lock
import itertools

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
    path_to_files: dict[str, FileInfo] = field(default_factory=lambda: defaultdict(FileInfo))

@dataclass
class ProcessOutput:
    lock: Lock = field(default_factory=Lock)
    buffer: str = ""


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

def launch_process(target: lldb.SBTarget, params: ExeParams, output: ProcessOutput) -> Optional[lldb.SBProcess]:
    li = lldb.SBLaunchInfo([])
    li.SetWorkingDirectory(params.working_dir)

    def read_into_output(process: lldb.SBProcess, output: ProcessOutput):
        while True:
            chunk = process.GetSTDOUT(2048)
            other_chunk = process.GetSTDERR(2048)

            with output.lock:
                output.buffer += chunk
                output.buffer += other_chunk

    process = target.Launch(li, lldb.SBError())

    read_thread = Thread(target=read_into_output, args=(process, output), daemon=True)
    read_thread.start()

    return process

def _var_name(value: lldb.SBValue) -> str:
    if not hasattr(value, 'cached_name'):
        # Vars with the same name will be declared across lines in the same frame, so we gotta do this
        id_ = value.GetAddress()
        setattr(value, 'cached_name', f'{value.name}:{id_}')

    return value.cached_name

def _get_vars(frame: lldb.SBFrame) -> list[lldb.SBValue]:
    if not hasattr(frame, 'cached_vars'):
        setattr(frame, 'cached_vars', list(frame.locals) + list(frame.arguments))

    return frame.cached_vars

def get_frame_var_names(frame: lldb.SBFrame) -> list[str]:
    if not hasattr(frame, 'var_names'):
        names = []

        for var in _get_vars(frame):
            names.append(_var_name(var))

        setattr(frame, 'var_names', names)

    return frame.var_names

def get_frame_var_values(frame: lldb.SBFrame, names: set[str]) -> dict[str, str]:
    if not hasattr(frame, 'var_name_to_value'):
        setattr(frame, 'var_name_to_value', {})

    name_to_value = {}

    for name in names:
        if name in frame.var_name_to_value:
            name_to_value[name] = frame.var_name_to_value[name]

    for var in _get_vars(frame):
        name = _var_name(var)

        if name in names and name not in name_to_value:
            var_str = str(var)
  
            frame.var_name_to_value[name] = var_str
            name_to_value[name] = var_str

    return name_to_value



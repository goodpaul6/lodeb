from typing import Optional

import sys

sys.path.insert(0, '/Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework/Resources/Python')

import lldb
import pygame
import imgui
import filedialpy
import OpenGL.GL as gl
import os.path
from dataclasses import dataclass 
from imgui.integrations.pygame import PygameRenderer

@dataclass
class LaunchParams:
    exe_path: str = ""
    working_dir: str = ""
    should_launch: bool = False


@dataclass
class ProcessMetadata:
    source_files: set[str]


def win_launch_executable(params: LaunchParams):
    imgui.begin('Launch Executable', True)

    exe_changed, params.exe_path = imgui.input_text('Executable Path', params.exe_path)

    imgui.same_line()
    if imgui.button('Browse'):
        params.exe_path = filedialpy.openFile()
        exe_changed = True

    wd_changed, params.working_dir = imgui.input_text('Working Directory', params.working_dir)

    imgui.same_line()
    if imgui.button('Browse'):
        params.working_dir = filedialpy.openDir()

    if exe_changed and params.working_dir == "":
        params.working_dir = os.path.dirname(params.exe_path)

    params.should_launch = imgui.button('Launch')

    imgui.end()


def launch_and_stop(lp: LaunchParams, dbg: lldb.SBDebugger) -> Optional[lldb.SBProcess]:
    target = dbg.CreateTargetWithFileAndArch(lp.exe_path, lldb.LLDB_ARCH_DEFAULT)

    if not target:
        return None

    bp = target.BreakpointCreateByLocation('test_parser.odin', 159)
    print(bp)

    process = target.LaunchSimple(None, None, lp.working_dir)

    return process


def get_process_metadata(process: lldb.SBProcess) -> ProcessMetadata:
    files = set()

    for mod in process.target.modules:
        for sym in mod:
            if sym.GetStartAddress().GetFileAddress() == 0:
                continue

            path = sym.GetStartAddress().GetLineEntry().GetFileSpec().fullpath

            if path: files.add(path)

    return ProcessMetadata(source_files=files)


def main():
    dbg = lldb.SBDebugger.Create()

    # No docs on how to do async mode so...
    dbg.SetAsync(False)

    pygame.init()

    size = 800, 600

    pygame.display.set_mode(size, pygame.DOUBLEBUF | pygame.OPENGL | pygame.RESIZABLE)

    # initilize imgui context (see documentation)
    imgui.create_context()
    impl = PygameRenderer()

    io = imgui.get_io()
    io.display_size = size

    launch_params = LaunchParams()
    process: Optional[lldb.SBProcess] = None
    process_metadata: Optional[ProcessMetadata] = None
    
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                sys.exit(0)

            impl.process_event(event)

        impl.process_inputs()

        # start new frame context
        imgui.new_frame()

        win_launch_executable(launch_params)

        if launch_params.should_launch:
            process = launch_and_stop(launch_params, dbg)

        if process and not process_metadata:
            process_metadata = get_process_metadata(process)
            print(process_metadata)


        gl.glClearColor(0, 0, 0, 0)
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)

        # pass all drawing comands to the rendering pipeline
        # and close frame context
        imgui.render()
        imgui.end_frame()

        impl.render(imgui.get_draw_data())

        pygame.display.flip()

if __name__ == '__main__':
    main()

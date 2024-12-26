from typing import Optional
from concurrent.futures import ThreadPoolExecutor

import sys
import os.path

sys.path.insert(0, '/Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework/Resources/Python')

import lldb

import pygame
import imgui
import filedialpy
import OpenGL.GL as gl
from imgui.integrations.pygame import PygameRenderer

import state
import debugger

def win_load_executable(st: state.State) -> bool:
    imgui.begin('Load Executable', True)

    exe_changed, st.exe_params.path = imgui.input_text('Executable Path', st.exe_params.exe_path)

    imgui.same_line()
    if imgui.button('Browse'):
        st.exe_params.exe_path = filedialpy.openFile()
        exe_changed = True

    wd_changed, st.exe_params.working_dir = imgui.input_text('Working Directory', st.exe_params.working_dir)

    imgui.same_line()
    if imgui.button('Browse'):
        st.exe_params.working_dir = filedialpy.openDir()

    if exe_changed and st.exe_params.working_dir == "":
        st.exe_params.working_dir = os.path.dirname(st.exe_params.exe_path)

    should_load = imgui.button('Load')

    imgui.end()

    return should_load


def win_target_metadata(st: state.State):
    if not st.target_metadata:
        return

    imgui.begin('Target Metadata')

    if not st.target_metadata.done():
        imgui.text('Loading symbols...')
        imgui.end()

        return

    md = st.target_metadata.result()

    changed, st.sym_search_text = imgui.input_text('Search Symbol', st.sym_search_text)

    for path, f in md.path_to_files.items():
        relevant_syms = [sym for sym in f.symbols if not st.sym_search_text or st.sym_search_text in sym.name]

        if not relevant_syms:
            continue

        if imgui.tree_node(path):
            for sym in relevant_syms:
                clicked, _ = imgui.selectable(sym.name, False)
                if clicked:
                    st.sym_to_open = state.LocatedSym(fi=f, path=path, sym=sym)

            imgui.tree_pop()


    imgui.end()


def win_source_file(st: state.State):
    if not st.source_file:
        return

    imgui.begin('Source File')

    imgui.text(f'Path: {st.source_file.path}')

    imgui.begin_child('Source Code', width=-1, height=-1, border=True)

    lines = st.source_file.text.split('\n')

    for idx, line in enumerate(lines):
        # Line number
        imgui.text_unformatted(f'{idx + 1}')

        imgui.same_line(60)

        imgui.text_unformatted(line)

        if st.source_file.scroll_to_line == idx + 1:
            imgui.set_scroll_here_y()
            st.source_file.scroll_to_line = None

    imgui.end_child()

    imgui.end()



def main():
    dbg = lldb.SBDebugger.Create()

    dbg.SetAsync(False)

    pygame.init()

    size = 800, 600

    pygame.display.set_mode(size, pygame.DOUBLEBUF | pygame.OPENGL | pygame.RESIZABLE)

    # initilize imgui context (see documentation)
    imgui.create_context()
    impl = PygameRenderer()

    io = imgui.get_io()
    io.display_size = size

    st = state.create()
    executor = ThreadPoolExecutor(max_workers=3)
    
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                sys.exit(0)

            impl.process_event(event)

        impl.process_inputs()

        # start new frame context
        imgui.new_frame()

        st.should_load = win_load_executable(st)         

        if st.should_load:
            st.target = debugger.create_target(st.dbg, st.exe_params)
            st.target_metadata = executor.submit(debugger.get_target_metadata_wait, st.target)

        win_target_metadata(st)

        if st.sym_to_open:
            sym_path = st.sym_to_open.path
            sym_line = st.sym_to_open.sym.addr.line_entry.line

            if st.source_file and sym_path == st.source_file.path:
                st.source_file.scroll_to_line = sym_line
            else:
                with open(sym_path) as f:
                    st.source_file = state.SourceFile(
                        path=sym_path,
                        text=f.read(),
                        scroll_to_line=sym_line
                    )

            st.sym_to_open = None

        win_source_file(st)

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

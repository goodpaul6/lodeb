from typing import Optional
from concurrent.futures import ThreadPoolExecutor, Future

import sys
import os.path
import re
import functools

sys.path.insert(0, '/Applications/Xcode.app/Contents/SharedFrameworks/LLDB.framework/Resources/Python')

import lldb

os.environ['PYGAME_HIDE_SUPPORT_PROMPT'] = '1'
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

    imgui.begin_child('Files')

    for path, f in md.path_to_files.items():
        relevant_syms = [sym for sym in f.symbols if not st.sym_search_text or st.sym_search_text in sym.name]

        if not relevant_syms:
            continue

        if imgui.tree_node(path):
            for sym in relevant_syms:
                clicked, _ = imgui.selectable(sym.name, False)
                if clicked:
                    st.loc_to_open = state.sym_loc(sym)

            imgui.tree_pop()

    imgui.end_child()

    imgui.end()


def win_source_file(st: state.State):
    if not st.source_file:
        return

    imgui.begin('Source File')

    imgui.text(f'Path: {st.source_file.path}')

    imgui.begin_child('Source Code', width=-1, height=-1, border=True)

    lines = st.source_file.text.split('\n')

    for idx, line in enumerate(lines):
        imgui.push_id(str(idx))

        loc = state.Loc(path=st.source_file.path, line=idx + 1)

        bp = st.loc_to_breakpoint.get(loc)

        if imgui.invisible_button('##gutter', width=16, height=16):
            # NOTE(Apaar): We avoid mutating things directly and instead signal
            # update to do the work so it can do cross-cutting things
            st.loc_to_toggle_breakpoint = loc

        imgui.same_line()
     
        if bp:
            # Draw filled circle if there is a breakpoint here
            draw_list = imgui.get_window_draw_list()
            cursor_pos = imgui.get_item_rect_min()
            draw_list.add_circle_filled(
                cursor_pos.x + 8, cursor_pos.y + 8,
                4,
                imgui.get_color_u32_rgba(1.0, 0.0, 0.0, 1.0)
            )

        imgui.same_line()

        text = f'{loc.line:5} {line}'

        if st.process and st.process.highlight_loc == loc:
            imgui.text_colored(text, 0.25, 0.5, 1.0, 1.0)
        else:
            imgui.text_unformatted(text)

        if st.source_file.scroll_to_line == idx + 1:
            imgui.set_scroll_here_y()
            st.source_file.scroll_to_line = None

        imgui.pop_id()

    imgui.end_child()

    imgui.end()


CSI_PATTERN = re.compile(r'\x1B\[[0-?]*[ -/]*[@-~]')
OSC_PATTERN = re.compile(r'\x1B\].*?(?:\x07|\x1B\\)', re.DOTALL)

def strip_ansi_codes(text: str) -> str:
    text = CSI_PATTERN.sub('', text)
    text = OSC_PATTERN.sub('', text)
    return text


def win_debug(st: state.State):
    if not st.target:
        # Can't debug anything without a target
        return

    imgui.begin('Debug')

    if not st.process:
        st.should_start = imgui.button('Start')
        imgui.end()

        return

    st.process.should_kill = imgui.button('Kill')

    if st.process.process.state == lldb.eStateStopped:
        st.process.should_step_in = imgui.button('Step In')
        imgui.same_line()
        st.process.should_step_over = imgui.button('Step Over')
        imgui.same_line()
        st.process.should_continue = imgui.button('Continue')

        thread = st.process.process.GetSelectedThread()
        selected_frame = st.process.selected_frame
        frames = thread and thread.frames

        if frames:
            imgui.text('Frames')

            imgui.begin_child('Frames', width=-1, height=100, border=True)

            for frame in frames:
                frame_info = strip_ansi_codes(str(frame))

                clicked, _ = imgui.selectable(frame_info, selected_frame and frame.GetFrameID() == selected_frame.GetFrameID())

                if clicked:
                    thread.SetSelectedFrame(frame.idx)

            imgui.end_child()

        if selected_frame and st.process.frame_var_id_to_str:
            imgui.text('Locals')

            imgui.begin_child('Locals', width=-1, height=200, border=True)

            if not st.process.frame_var_id_to_str.done():
                imgui.text('Loading locals (LLDB is slow)...')
            else:
                for var_id, s in st.process.frame_var_id_to_str.result():
                    clicked, _ = imgui.selectable(s, False)

            imgui.end_child() 
    else:
        st.process.selected_frame = None

    imgui.end()

def win_output(st: state.State):
    if not st.output:
        return

    imgui.begin('Process Output')
    imgui.begin_child('Text', width=-1, height=-1, border=True)

    with st.output.lock:
        text = strip_ansi_codes(st.output.buffer)

    imgui.text_unformatted(text)

    imgui.end_child()
    imgui.end()


def update(st: state.State, executor: ThreadPoolExecutor):
    if st.should_load:
        st.target = debugger.create_target(st.dbg, st.exe_params)
        st.target_metadata = executor.submit(debugger.get_target_metadata_wait, st.target)

        st.should_load = False

    if st.loc_to_open:
        path = st.loc_to_open.path
        line = st.loc_to_open.line

        if st.source_file and path == st.source_file.path:
            st.source_file.scroll_to_line = line
        else:
            with open(path) as f:
                st.source_file = state.SourceFile(
                    path=path,
                    text=f.read(),
                    scroll_to_line=line
                )

        st.loc_to_open = None

    if st.target and st.loc_to_toggle_breakpoint:
        loc = st.loc_to_toggle_breakpoint
        
        if loc in st.loc_to_breakpoint:
            prev_bp = st.loc_to_breakpoint[loc]  
            st.target.BreakpointDelete(prev_bp.id)

            st.loc_to_breakpoint.pop(loc, None)
        else:
            bp = debugger.create_breakpoint_by_file_line(st.target, loc.path, loc.line)
            
            if not bp.IsValid():
                print('Invalid breakpoint')
            else:
                st.loc_to_breakpoint[loc] = bp

        st.loc_to_toggle_breakpoint = None
    else:
        st.loc_to_toggle_breakpoint = None

    if not st.process and st.should_start:
        # Make a new output object (discard previous output)
        st.output = debugger.ProcessOutput()

        process = debugger.launch_process(st.target, st.exe_params, st.output)

        st.process = state.ProcessState(process=process)
        st.should_start = False

    if st.process:
        if st.process.should_kill:
            # Seems like we must continue before we can kill (assuming it is stopped)
            st.process.process.Continue()
            st.process.process.Kill()

            st.process.should_kill = False

        pstate = st.process.process.state

        if pstate == lldb.eStateStopped:
            thread = st.process.process.GetSelectedThread()
            frame = thread and thread.GetSelectedFrame()
            line_entry = frame and frame.line_entry

            if line_entry:
                new_loc = state.Loc(path=line_entry.file.fullpath, line=line_entry.line)

                if st.process.highlight_loc != new_loc:
                    st.process.highlight_loc = new_loc
                    # Navigate the source view to this loc
                    st.loc_to_open = new_loc

            if thread:
                if st.process.should_step_in:
                    thread.StepInto()
                    st.process.selected_frame = None

                if st.process.should_step_over:
                    thread.StepOver() 
                    st.process.selected_frame = None

                if st.process.should_continue:
                    st.process.process.Continue()
                    st.process.selected_frame = None

                lldb_selected_frame = thread.GetSelectedFrame()

                # Update it as needed but cache otherwise
                if not st.process.selected_frame or lldb_selected_frame.GetFrameID() != st.process.selected_frame.GetFrameID():
                    st.process.selected_frame = lldb_selected_frame

                    # TODO(Apaar): When I run this on another thread, the entire program goes wonky. Is LLDB process/frame API not
                    # thread safe?
                    st.process.frame_var_id_to_str = Future()
                    st.process.frame_var_id_to_str.set_result(debugger.get_frame_var_strs(lldb_selected_frame))

        elif pstate in [lldb.eStateExited, lldb.eStateDetached, lldb.eStateUnloaded] or not st.process.process.is_alive:
            st.process = None
        else:
            st.process.highlight_loc = None
            st.process.selected_frame = None


def main():
    dbg = lldb.SBDebugger.Create()

    dbg.SetAsync(False)

    pygame.init()

    size = 800, 600

    pygame.display.set_caption('Lodeb')
    pygame.display.set_mode(size, pygame.DOUBLEBUF | pygame.OPENGL | pygame.RESIZABLE)

    # initilize imgui context (see documentation)
    imgui.create_context()
    impl = PygameRenderer()

    io = imgui.get_io()
    io.display_size = size

    st = state.create()

    try:
        state.load(st, 'lodeb.json')
    except Exception as e:
        print(e)

    executor = ThreadPoolExecutor(max_workers=3)
    
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                sys.exit(0)

            impl.process_event(event)

        update(st, executor)

        impl.process_inputs()

        # start new frame context
        imgui.new_frame()

        st.should_load = win_load_executable(st)
        win_target_metadata(st)
        win_source_file(st)
        win_debug(st)
        win_output(st)

        gl.glClearColor(0, 0, 0, 0)
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)

        # pass all drawing comands to the rendering pipeline
        # and close frame context
        imgui.render()
        imgui.end_frame()

        impl.render(imgui.get_draw_data())

        pygame.display.flip()

        state.store(st, 'lodeb.json')

if __name__ == '__main__':
    main()

# CMAKE generated file: DO NOT EDIT!
# Generated by CMake Version 3.30
cmake_policy(SET CMP0009 NEW)

# scaffold_sources at CMakeLists.txt:11 (file)
file(GLOB NEW_GLOB LIST_DIRECTORIES true "/Users/apaar/projects/lodeb/cpp/scaffold/builtin/*.cpp")
set(OLD_GLOB
  "/Users/apaar/projects/lodeb/cpp/scaffold/builtin/DemoLayer.cpp"
  "/Users/apaar/projects/lodeb/cpp/scaffold/builtin/InputInfoLayer.cpp"
  "/Users/apaar/projects/lodeb/cpp/scaffold/builtin/ProfilerLayer.cpp"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/Users/apaar/projects/lodeb/cpp/build/CMakeFiles/cmake.verify_globs")
endif()

# scaffold_sources at CMakeLists.txt:11 (file)
file(GLOB NEW_GLOB LIST_DIRECTORIES true "/Users/apaar/projects/lodeb/cpp/scaffold/source/*.cpp")
set(OLD_GLOB
  "/Users/apaar/projects/lodeb/cpp/scaffold/source/Application.cpp"
  "/Users/apaar/projects/lodeb/cpp/scaffold/source/Input.cpp"
  "/Users/apaar/projects/lodeb/cpp/scaffold/source/Marker.cpp"
  "/Users/apaar/projects/lodeb/cpp/scaffold/source/Profiler.cpp"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/Users/apaar/projects/lodeb/cpp/build/CMakeFiles/cmake.verify_globs")
endif()

# scaffold_sources at CMakeLists.txt:11 (file)
file(GLOB NEW_GLOB LIST_DIRECTORIES true "/Users/apaar/projects/lodeb/cpp/vendor/glad/src/gl.c")
set(OLD_GLOB
  "/Users/apaar/projects/lodeb/cpp/vendor/glad/src/gl.c"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/Users/apaar/projects/lodeb/cpp/build/CMakeFiles/cmake.verify_globs")
endif()

# scaffold_sources at CMakeLists.txt:11 (file)
file(GLOB NEW_GLOB LIST_DIRECTORIES true "/Users/apaar/projects/lodeb/cpp/vendor/imgui/*.cpp")
set(OLD_GLOB
  "/Users/apaar/projects/lodeb/cpp/vendor/imgui/imgui.cpp"
  "/Users/apaar/projects/lodeb/cpp/vendor/imgui/imgui_demo.cpp"
  "/Users/apaar/projects/lodeb/cpp/vendor/imgui/imgui_draw.cpp"
  "/Users/apaar/projects/lodeb/cpp/vendor/imgui/imgui_impl_glfw.cpp"
  "/Users/apaar/projects/lodeb/cpp/vendor/imgui/imgui_impl_opengl3.cpp"
  "/Users/apaar/projects/lodeb/cpp/vendor/imgui/imgui_tables.cpp"
  "/Users/apaar/projects/lodeb/cpp/vendor/imgui/imgui_widgets.cpp"
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/Users/apaar/projects/lodeb/cpp/build/CMakeFiles/cmake.verify_globs")
endif()

# scaffold_sources at CMakeLists.txt:11 (file)
file(GLOB NEW_GLOB LIST_DIRECTORIES true "/Users/apaar/projects/lodeb/cpp/vendor/tinygltf/tinygltf_impl.cpp")
set(OLD_GLOB
  )
if(NOT "${NEW_GLOB}" STREQUAL "${OLD_GLOB}")
  message("-- GLOB mismatch!")
  file(TOUCH_NOCREATE "/Users/apaar/projects/lodeb/cpp/build/CMakeFiles/cmake.verify_globs")
endif()

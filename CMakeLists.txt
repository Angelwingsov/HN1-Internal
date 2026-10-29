cmake_minimum_required(VERSION 3.16)
project(HN1Cheat LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/core/hooks.cpp
    src/game/ue_types.cpp
    src/game/engine.cpp
    src/features/noclip.cpp
    src/features/stealth.cpp
    src/features/camera.cpp
    src/features/esp.cpp
    src/features/features.cpp
    src/gui/menu.cpp
    deps/kiero/kiero.cpp
    deps/imgui/imgui.cpp
    deps/imgui/imgui_draw.cpp
    deps/imgui/imgui_tables.cpp
    deps/imgui/imgui_widgets.cpp
    deps/imgui/imgui_impl_win32.cpp
    deps/imgui/imgui_impl_dx11.cpp
    deps/minhook/hook.c
    deps/minhook/buffer.c
    deps/minhook/trampoline.c
    deps/minhook/hde64.c
)

add_library(HN1Cheat SHARED ${SOURCES})

target_include_directories(HN1Cheat PRIVATE
    src
    deps
    deps/imgui
    deps/kiero
    deps/minhook
)

target_link_libraries(HN1Cheat PRIVATE
    d3d11
    dxgi
    user32
)

set_target_properties(HN1Cheat PROPERTIES
    OUTPUT_NAME "HN1Cheat"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

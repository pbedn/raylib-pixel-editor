cmake_minimum_required(VERSION 3.16)

project(raylib_pixel_editor LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

add_executable(
  pixel
  src/pixel-editor.c
  src/pixel_core.c
  src/pixel_ui_logic.c
)

target_include_directories(pixel PRIVATE "${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/src")
target_compile_definitions(pixel PRIVATE _POSIX_C_SOURCE=200809L)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(pixel PRIVATE -Wall -Wextra -O2)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/lib/libraylib.a")
  target_link_libraries(pixel PRIVATE "${CMAKE_SOURCE_DIR}/lib/libraylib.a")
endif()

if(UNIX AND NOT APPLE)
  target_link_libraries(pixel PRIVATE m dl pthread GL rt X11)
endif()

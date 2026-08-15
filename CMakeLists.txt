cmake_minimum_required(VERSION 3.14)
project(TracerNator VERSION 0.1.0 LANGUAGES C CXX)

if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
endif()
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    rapidjson
    GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
    GIT_TAG        24b5e7a8b27f42fa16b96fc70aade9106cf7102f
)

FetchContent_GetProperties(rapidjson)
if(NOT rapidjson_POPULATED)
    FetchContent_Populate(rapidjson)
endif()

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        2c980bb59875b0d32144a71867fbdebb2f77cd20
)

FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()

# Core Logic
add_library(Core
    src/camera.cpp
    src/matrix.cpp
    src/mesh.cpp
    src/scene.cpp
    src/renderer.cpp
    src/light.cpp
    src/material.cpp
    src/texture.cpp
    src/thread_pool.cpp
    src/aabb.cpp
)


target_include_directories(Core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${rapidjson_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
)

function(add_app NAME SRC)
    add_executable(${NAME} ${SRC})
    target_link_libraries(${NAME} PRIVATE Core)
endfunction()

# Executables
add_app(program apps/main.cpp)
add_app(hw_02   apps/HW_02.cpp)
add_app(hw_03   apps/HW_03.cpp)
add_app(hw_05   apps/HW_05.cpp)
add_app(hw_06   apps/HW_06.cpp)
add_app(hw_08   apps/HW_08.cpp)
add_app(hw_09   apps/HW_09.cpp)
add_app(hw_10   apps/HW_10.cpp)
cmake_minimum_required(VERSION 3.28)
project(MyWorkspace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(/Zc:__cplusplus)

set(ENGINE "game-system")
set(APP_NAME "game-app")

add_subdirectory(${ENGINE})
add_subdirectory(${APP_NAME})

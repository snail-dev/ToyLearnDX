cmake_minimum_required(VERSION 3.14)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("DX11 Demo Project")

option(WIN7_SYSTEM_SUPPORT "Windows7 users need to select this option!" OFF)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

add_subdirectory("ImGui")
set_target_properties(ImGui PROPERTIES FOLDER "ImGui")

add_subdirectory("DXInit")
add_subdirectory("Triangle")
add_subdirectory("Cube")
add_subdirectory("ImGui-Cube")
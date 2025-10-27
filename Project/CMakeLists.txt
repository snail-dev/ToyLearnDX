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
add_subdirectory("Lighting")
add_subdirectory("Interoperability")
add_subdirectory("TextureMapping")
add_subdirectory("Camera")
add_subdirectory("Blending")
add_subdirectory("DepthAndStenciling")
add_subdirectory("Living Without FX11")
add_subdirectory("Depth Test")
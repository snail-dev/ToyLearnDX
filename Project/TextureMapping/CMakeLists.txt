cmake_minimum_required(VERSION 3.14)

add_compile_definitions(UNICODE _UNICODE)
if(WIN7_SYSTEM_SUPPORT MATCHES ON)
    add_compile_definitions(_WIN32_WINNT=0x601)
endif()

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

aux_source_directory(. DIR_SRCS)
file(GLOB HEADER_FILES ./*.h)
file(GLOB HLSL_FILES HLSL/*.hlsl HLSL/*.hlsli)

foreach(HLSL_FILE ${HLSL_FILES})
    get_filename_component(HLSL_FDIR ${HLSL_FILE} DIRECTORY)
    get_filename_component(HLSL_FBASENAME_WE ${HLSL_FILE} NAME_WE)
    string (CONCAT HLSL_FNAME_WE ${HLSL_FDIR} / ${HLSL_FBASENAME_WE})

    string(LENGTH ${HLSL_FBASENAME_WE} LEN_FNAME_WE)

    math(EXPR LEN_FNAME_WE "${LEN_FNAME_WE}-2")
    string(SUBSTRING ${HLSL_FBASENAME_WE} ${LEN_FNAME_WE} 2 ENTRY_POINT)
    string(TOLOWER ${ENTRY_POINT} SHADER_TYPE)

    if("${SHADER_TYPE}" STREQUAL "vs")
        set(SHADER_TYPE "Vertex")
    elseif("${SHADER_TYPE}" STREQUAL "hs")
        set(SHADER_TYPE "Hull")
    elseif("${SHADER_TYPE}" STREQUAL "ds")
        set(SHADER_TYPE "Domain")
    elseif("${SHADER_TYPE}" STREQUAL "gs")
        set(SHADER_TYPE "Geometry")
    elseif("${SHADER_TYPE}" STREQUAL "ps")
        set(SHADER_TYPE "Pixel")
    elseif("${SHADER_TYPE}" STREQUAL "cs")
        set(SHADER_TYPE "Compute") 
    endif()

    set_source_files_properties(${HLSL_FILE} PROPERTIES 
                                 VS_SHADER_OBJECT_FILE_NAME HLSL/${HLSL_FBASENAME_WE}.cso
                                 VS_SHADER_TYPE ${SHADER_TYPE}
                                 VS_SHADER_MODEL 5.0
                                 VS_SHADER_ENTRYPOINT ${ENTRY_POINT}
                                 VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:true>
                                 VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true> )
endforeach()

add_executable(TextureMapping WIN32 ${DIR_SRCS} ${HEADER_FILES} ${HLSL_FILES})
source_group("HLSL Files" FILES ${HLSL_FILES})

target_link_libraries(TextureMapping d3d11.lib dxgi.lib dxguid.lib D3DCompiler.lib d2d1.lib dwrite.lib winmm.lib)

# ImGui
target_link_libraries(TextureMapping ImGui)
file(COPY imgui.ini DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Texture
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/Texture)
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Texture DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

set_target_properties(TextureMapping PROPERTIES OUTPUT_NAME "TextureMapping")

set_target_properties(TextureMapping PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(TextureMapping PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR})
set_target_properties(TextureMapping PROPERTIES FOLDER "TextureMapping")
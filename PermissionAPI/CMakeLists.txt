cmake_minimum_required(VERSION 3.21)

project(PermissionAPI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Release)

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/x64/)
set(TARGET_DIR ${CMAKE_SOURCE_DIR}/x64/Release/)

file(GLOB_RECURSE SRC_FILES_DIR
    ${PROJECT_SOURCE_DIR}/*.c
    ${PROJECT_SOURCE_DIR}/*.cpp
    ${PROJECT_SOURCE_DIR}/*.h
)

include_directories(
    ${CMAKE_SOURCE_DIR}/SDK/Header
	${CMAKE_SOURCE_DIR}/SDK/Header/third-party
)

link_directories(
    ${CMAKE_SOURCE_DIR}/SDK/Lib
)

add_definitions(
    -D"NDEBUG"
    -D"NOMINMAX"
    -D"TEMPLATE_EXPORTS"
    -D"UNICODE"
    -D"WIN32_LEAN_AND_MEAN"
    -D"_AMD64_"
    -D"_CRT_SECURE_NO_WARNINGS"
    -D"_UNICODE"
    -D"_USRDLL"
    -D"_WINDLL"
    -D"_WINDOWS"
    -D"PERMAPI_EXPORTS"
)

add_compile_options(
    /diagnostics:column
    /EHsc
    /FC
    /GL
    /MD
    /nologo
    /permissive-
    /sdl
    /utf-8
    /Zc:inline
    /Zi
)

add_link_options(
    /DEBUG
    /DELAYLOAD:"bedrock_server.dll"
    /DLL
    /IGNORE:4199
    /INCREMENTAL:NO
    /LTCG:INCREMENTAL
    /MANIFESTUAC:NO
    /MACHINE:X64
    /NOLOGO
    /OPT:ICF
    /OPT:REF
    /SUBSYSTEM:CONSOLE
)

add_library(PermissionAPI SHARED ${SRC_FILES_DIR})

target_link_libraries(PermissionAPI 
	"${CMAKE_SOURCE_DIR}/SDK/Lib/LiteLoader.lib"
	"${CMAKE_SOURCE_DIR}/SDK/Lib/bedrock_server_api.lib"
	"${CMAKE_SOURCE_DIR}/SDK/Lib/bedrock_server_var.lib"
	"${CMAKE_SOURCE_DIR}/SDK/Lib/SymDBHelper.lib"
)
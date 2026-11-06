cmake_minimum_required(VERSION 3.21)
project(OpenRGBASUSLighting VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(NOT WIN32 OR NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Use MSVC x64 for the supported Windows OpenRGB build")
endif()

set(OPENRGB_SOURCE_DIR "" CACHE PATH "Path to the OpenRGB 1.1 source tree")
if(NOT OPENRGB_SOURCE_DIR OR NOT EXISTS "${OPENRGB_SOURCE_DIR}/OpenRGBPluginInterface.h")
    message(FATAL_ERROR "Set OPENRGB_SOURCE_DIR to a complete OpenRGB 1.1 source tree")
endif()

find_package(Qt6 6.8.3 EXACT REQUIRED COMPONENTS Core Gui Widgets)

add_library(OpenRGBASUSLighting MODULE
    AsusLightingPlugin.cpp
    AsusLightingPlugin.h
    LightingEncoding.cpp
    AsioSuperIo.cpp
    AsioSuperIo.h
    plugin.json
)

target_include_directories(OpenRGBASUSLighting PRIVATE
    "${OPENRGB_SOURCE_DIR}"
    "${OPENRGB_SOURCE_DIR}/RGBController"
    "${OPENRGB_SOURCE_DIR}/dependencies/json"
)
target_link_libraries(OpenRGBASUSLighting PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
target_compile_definitions(OpenRGBASUSLighting PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
set_target_properties(OpenRGBASUSLighting PROPERTIES
    PREFIX ""
    OUTPUT_NAME "OpenRGBASUSLighting"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

option(ASUS_LIGHTING_BUILD_CHECKS "Build checks that do not send hardware commands" OFF)
if(ASUS_LIGHTING_BUILD_CHECKS)
    add_executable(ASUSLightingChecks checks/PluginChecks.cpp LightingEncoding.cpp)
    get_target_property(ASUS_PLUGIN_INCLUDES OpenRGBASUSLighting INCLUDE_DIRECTORIES)
    target_include_directories(ASUSLightingChecks PRIVATE ${ASUS_PLUGIN_INCLUDES} "${CMAKE_CURRENT_SOURCE_DIR}")
    target_link_libraries(ASUSLightingChecks PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
    target_compile_definitions(ASUSLightingChecks PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

# Independent DLL with its own plugin ID and device panel.
add_subdirectory(plugins/CorsairVoid)

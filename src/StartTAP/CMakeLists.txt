cmake_minimum_required(VERSION 3.15)
project(StartTAP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Windows SDK and WinRT
find_package(PkgConfig REQUIRED)

# Set compiler flags for WinRT
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20 /EHsc")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_WIN32_WINNT=0x0A00") # Windows 10

# Source files
set(SOURCES
    dllmain.cpp
    VisualTreeWatcher.cpp
)

# Headers
set(HEADERS
    framework.h
    VisualTreeWatcher.h
    helpers.h
)

# Create DLL
add_library(StartTAP SHARED ${SOURCES} ${HEADERS})

# Set output name
set_target_properties(StartTAP PROPERTIES
    OUTPUT_NAME "StartTAP"
    PREFIX ""
)

# Link libraries
target_link_libraries(StartTAP
    windowsapp
    kernel32
    user32
    advapi32
)

# Include directories
target_include_directories(StartTAP PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler definitions
target_compile_definitions(StartTAP PRIVATE
    _WINDOWS
    _USRDLL
    STARTTAP_EXPORTS
)

# Use def file for exports
set_target_properties(StartTAP PROPERTIES
    LINK_FLAGS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/exports.def"
)

# Install rules
install(TARGETS StartTAP
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
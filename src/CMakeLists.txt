# CMakeLists.txt for TranslucentSM - Windows Start Menu Transparency Tool
# Version: 1.0.0
# Author: mc-yzy15
# Email: yingmoliuguang@yeah.net
# Date: 2026-01-09

cmake_minimum_required(VERSION 3.15)
project(TranslucentSM C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Ensure support for Visual Studio
if(MSVC)
    # Set Visual Studio specific options
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4")
    # Enable multi-processor compilation
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    # Set output directories, ensure Debug and Release both output to bin directory
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG} CONFIG)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/bin)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/bin)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/lib)
    endforeach()
endif()

# Windows specific configuration
if(WIN32)
    # Add Windows SDK and necessary libraries
    add_definitions(-D_WIN32_WINNT=0x0A00) # Windows 10 and above
    add_definitions(-DWINVER=0x0A00)
    # Use ANSI instead of Unicode to avoid type mismatch issues
    remove_definitions(-DUNICODE -D_UNICODE)

    # Link necessary system libraries
    set(EXTRA_LIBS
        psapi
        kernel32
        user32
        gdi32
        advapi32
        shell32
        shlwapi
        wininet
        dwmapi
        comctl32
    )
endif()

# Set include directories, ensure header files can be found
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Main program source files
set(MAIN_SOURCES
    main.c
    translucentsm.c
)

# Main program header files
set(MAIN_HEADERS
    translucentsm.h
)

# DLL source files
set(DLL_SOURCES
    translucentsmdll.c
)

# Create DLL project
add_library(TranslucentSM SHARED ${DLL_SOURCES})
set_target_properties(TranslucentSM PROPERTIES
    OUTPUT_NAME "TranslucentSM"
    SUFFIX ".dll"
    PREFIX ""
)

target_link_libraries(TranslucentSM PRIVATE ${EXTRA_LIBS})

# Create executable
add_executable(TranslucentSMApp WIN32 ${MAIN_SOURCES} ${MAIN_HEADERS})
set_target_properties(TranslucentSMApp PROPERTIES
    OUTPUT_NAME "TranslucentSM"
)

# Link libraries
target_link_libraries(TranslucentSMApp PRIVATE ${EXTRA_LIBS})

# Add dependencies
add_dependencies(TranslucentSMApp TranslucentSM)

# Copy DLL to executable directory
add_custom_command(TARGET TranslucentSMApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:TranslucentSM> $<TARGET_FILE_DIR:TranslucentSMApp>)

# Add StartTAP subdirectory for the new DLL implementation
add_subdirectory(StartTAP)

# Install rules
install(TARGETS TranslucentSM TranslucentSMApp
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
)

# Also install StartTAP DLL
install(TARGETS StartTAP
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
)

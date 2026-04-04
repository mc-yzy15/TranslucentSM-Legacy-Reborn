# CMakeLists.txt for the legacy TranslucentSM fallback
cmake_minimum_required(VERSION 3.15)
project(TranslucentSM C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(TRANSLUCENTSM_ENABLE_STARTTAP_CMAKE "Attempt to build StartTAP from CMake (unsupported)" OFF)

if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W4 /MP")
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG} CONFIG_UPPER)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/lib)
    endforeach()
endif()

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0A00)
    add_definitions(-DWINVER=0x0A00)
    remove_definitions(-DUNICODE -D_UNICODE)

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

if(TRANSLUCENTSM_ENABLE_STARTTAP_CMAKE)
    message(FATAL_ERROR "StartTAP is built through MSBuild only. Use start.sln or src/StartTAP/StartTAP.vcxproj after restoring NuGet packages.")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(MAIN_SOURCES
    main.c
    translucentsm.c
)

set(MAIN_HEADERS
    translucentsm.h
)

set(DLL_SOURCES
    translucentsmdll.c
)

add_library(TranslucentSM SHARED ${DLL_SOURCES})
set_target_properties(TranslucentSM PROPERTIES
    OUTPUT_NAME "TranslucentSM"
    SUFFIX ".dll"
    PREFIX ""
)

target_link_libraries(TranslucentSM PRIVATE ${EXTRA_LIBS})

add_executable(TranslucentSMApp WIN32 ${MAIN_SOURCES} ${MAIN_HEADERS})
set_target_properties(TranslucentSMApp PROPERTIES
    OUTPUT_NAME "TranslucentSM"
)

target_link_libraries(TranslucentSMApp PRIVATE ${EXTRA_LIBS})
add_dependencies(TranslucentSMApp TranslucentSM)

add_custom_command(TARGET TranslucentSMApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:TranslucentSM> $<TARGET_FILE_DIR:TranslucentSMApp>)

install(TARGETS TranslucentSM TranslucentSMApp
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
)

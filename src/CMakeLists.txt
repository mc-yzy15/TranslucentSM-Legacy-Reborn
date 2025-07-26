cmake_minimum_required(VERSION 3.15)
project(TranslucentSM)

# 添加Qt5依赖
find_package(Qt5 COMPONENTS Widgets Core Gui REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 设置Qt5模块
set(QT_MODULES Qt5::Widgets Qt5::Core Qt5::Gui)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows特定配置
if(WIN32)
    # 设置输出目录
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

    # 添加Windows SDK和必要的库
    add_definitions(-D_WIN32_WINNT=0x0A00) # Windows 10及以上
    add_definitions(-DWINVER=0x0A00)
    add_definitions(-DUNICODE -D_UNICODE)

    # 链接必要的系统库
    set(EXTRA_LIBS psapi kernel32 user32 gdi32 advapi32 shell32 dwmapi)
endif()

# 添加源文件
set(SOURCES
    TranslucentSM.cpp
    main.cpp
    installer.cpp
    MainWindow.cpp
)

# 添加头文件
set(HEADERS
    TranslucentSM.h
    MainWindow.h
    installer.h
)

# 创建DLL项目
add_library(TranslucentSM SHARED TranslucentSMDLL.cpp)
set_target_properties(TranslucentSM PROPERTIES
    OUTPUT_NAME "TranslucentSM"
    SUFFIX ".dll"
    PREFIX ""
)

target_link_libraries(TranslucentSM dwmapi user32 gdi32 shell32)

# 创建可执行文件
add_executable(TranslucentSMApp ${SOURCES} ${HEADERS})
set_target_properties(TranslucentSMApp PROPERTIES
    OUTPUT_NAME "TranslucentSM"
)

# 链接库
if(WIN32)
    # 链接Qt5库和其他必要库
    target_link_libraries(TranslucentSMApp ${QT_MODULES} ${EXTRA_LIBS})
endif()

# 添加依赖关系
add_dependencies(TranslucentSMApp TranslucentSM)

# 复制DLL到可执行文件目录
add_custom_command(TARGET TranslucentSMApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:TranslucentSM> $<TARGET_FILE_DIR:TranslucentSMApp>)

# 安装规则
install(TARGETS TranslucentSM TranslucentSMApp
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
)
install(FILES $<TARGET_PDB_FILE:TranslucentSM> DESTINATION bin OPTIONAL)
install(FILES $<TARGET_PDB_FILE:TranslucentSMApp> DESTINATION bin OPTIONAL)

# 安装规则

install(FILES $<TARGET_PDB_FILE:TranslucentSM> DESTINATION bin OPTIONAL)
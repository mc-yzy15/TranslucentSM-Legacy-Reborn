# 创建并进入build目录
if (-not (Test-Path -Path "build")) {
    New-Item -ItemType Directory -Name "build"
}
Set-Location -Path build

# 运行CMake配置（针对MinGW Makefiles生成器）
cmake -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_C_COMPILER="C:\Qt\6.9.1\mingw_64\bin/gcc.exe" `
  -DCMAKE_CXX_COMPILER="C:\Qt\6.9.1\mingw_64\bin/g++.exe" `
  -DCMAKE_VERBOSE_MAKEFILE=ON `
  -DCMAKE_MAKE_PROGRAM="C:\Qt\6.9.1\mingw_64\bin\mingw32-make.exe" `
  -DQt6_DIR="C:\Qt\6.9.1\mingw_64\lib\cmake\Qt6" `
  ../src

# 执行构建
  # PowerShell中获取逻辑处理器数量
  $processorCount = (Get-CimInstance -ClassName Win32_Processor).NumberOfLogicalProcessors
  & "C:\Qt\6.9.1\mingw_64\bin\mingw32-make.exe" -j $processorCount
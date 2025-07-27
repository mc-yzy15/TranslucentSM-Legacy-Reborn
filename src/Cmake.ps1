# 创建并进入build目录
if (-not (Test-Path -Path "build")) {
    New-Item -ItemType Directory -Name "build"
}
Set-Location -Path build

# 运行CMake配置（针对MinGW Makefiles生成器）
cmake -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_C_COMPILER=gcc `
  -DCMAKE_CXX_COMPILER=g++ `
  -DCMAKE_VERBOSE_MAKEFILE=ON `
  -DCMAKE_MAKE_PROGRAM=mingw32-make `
  -DQt5_DIR="C:/Qt/6.9.1/mingw_64/lib/cmake/Qt6" `
  ../src

# 执行构建
# PowerShell 中没有 $(nproc)，使用 Get-WmiObject 获取逻辑处理器数量
$processorCount = (Get-WmiObject -Class Win32_Processor).NumberOfLogicalProcessors
mingw32-make -j$processorCount
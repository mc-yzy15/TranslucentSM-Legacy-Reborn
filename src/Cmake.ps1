# 创建并进入build目录
# 清理并重建build目录
if (Test-Path -Path "build") {
    echo "正在清理旧构建目录..."
    Remove-Item -Path "build" -Recurse -Force
}
New-Item -ItemType Directory -Name "build"
Set-Location -Path build

# 设置MinGW环境变量
$env:PATH += ";C:/Qt/6.9.1/mingw_64/bin"
echo "MinGW路径已添加到环境变量: $env:PATH"

# 运行CMake配置（启用详细调试输出）
echo "正在运行CMake配置..."
cmake -G "MinGW Makefiles" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_C_COMPILER="C:/Qt/6.9.1/mingw_64/bin/gcc.exe" `
  -DCMAKE_CXX_COMPILER="C:/Qt/6.9.1/mingw_64/bin/g++.exe" `
  -DCMAKE_VERBOSE_MAKEFILE=ON `
  -DCMAKE_MAKE_PROGRAM="C:/Qt/6.9.1/mingw_64/bin/mingw32-make.exe" `
  -DQt6_DIR="C:/Qt/6.9.1/mingw_64/lib/cmake/Qt6" `
  --debug-output `
  ../src

if ($LASTEXITCODE -ne 0) {
  echo "CMake配置失败，错误代码: $LASTEXITCODE"
  exit $LASTEXITCODE
}

# 执行构建
# PowerShell中获取逻辑处理器数量
$processorCount = (Get-CimInstance -ClassName Win32_Processor).NumberOfLogicalProcessors
echo "使用 $processorCount 个处理器进行构建..."
& "C:/Qt/6.9.1/mingw_64/bin/mingw32-make.exe" -j $processorCount

if ($LASTEXITCODE -ne 0) {
  echo "构建失败，错误代码: $LASTEXITCODE"
  exit $LASTEXITCODE
}

echo "构建成功完成！"
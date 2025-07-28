# 检查并删除build目录
if (Test-Path -Path "build") {
    Write-Host "正在删除build目录..."
    Remove-Item -Path "build" -Recurse -Force
}
# 创建build目录并进入
New-Item -ItemType Directory -Name "build"
Set-Location -Path "build"

# 设置MinGW路径
$env:PATH += ";C:/Qt/6.9.1/mingw_64/bin"
Write-Host "MinGW路径已添加到系统路径: $env:PATH"

# 运行CMake配置，启用详细输出
Write-Host "正在运行CMake配置..."
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
  Write-Host "CMake配置失败，错误代码: $LASTEXITCODE"
  exit $LASTEXITCODE
}

# 执行编译
# 在PowerShell中获取逻辑处理器数量
$processorCount = (Get-CimInstance -ClassName Win32_Processor).NumberOfLogicalProcessors
Write-Host "使用 $processorCount 个线程进行编译..."
& "C:/Qt/6.9.1/mingw_64/bin/mingw32-make.exe" -j $processorCount

if ($LASTEXITCODE -ne 0) {
  Write-Host "编译失败，错误代码: $LASTEXITCODE"
  exit $LASTEXITCODE
}

Write-Host "Build completed successfully!"

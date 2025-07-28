# ��鲢ɾ��buildĿ¼
if (Test-Path -Path "build") {
    Write-Host "����ɾ��buildĿ¼..."
    Remove-Item -Path "build" -Recurse -Force
}
# ����buildĿ¼������
New-Item -ItemType Directory -Name "build"
Set-Location -Path "build"

# ����MinGW·��
$env:PATH += ";C:/Qt/6.9.1/mingw_64/bin"
Write-Host "MinGW·������ӵ�ϵͳ·��: $env:PATH"

# ����CMake���ã�������ϸ���
Write-Host "��������CMake����..."
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
  Write-Host "CMake����ʧ�ܣ��������: $LASTEXITCODE"
  exit $LASTEXITCODE
}

# ִ�б���
# ��PowerShell�л�ȡ�߼�����������
$processorCount = (Get-CimInstance -ClassName Win32_Processor).NumberOfLogicalProcessors
Write-Host "ʹ�� $processorCount ���߳̽��б���..."
& "C:/Qt/6.9.1/mingw_64/bin/mingw32-make.exe" -j $processorCount

if ($LASTEXITCODE -ne 0) {
  Write-Host "����ʧ�ܣ��������: $LASTEXITCODE"
  exit $LASTEXITCODE
}

Write-Host "Build completed successfully!"

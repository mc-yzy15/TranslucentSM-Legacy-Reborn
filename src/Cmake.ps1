# ����������buildĿ¼
# �������ؽ�buildĿ¼
if (Test-Path -Path "build") {
    echo "���������ɹ���Ŀ¼..."
    Remove-Item -Path "build" -Recurse -Force
}
New-Item -ItemType Directory -Name "build"
Set-Location -Path build

# ����MinGW��������
$env:PATH += ";C:/Qt/6.9.1/mingw_64/bin"
echo "MinGW·�������ӵ���������: $env:PATH"

# ����CMake���ã�������ϸ���������
echo "��������CMake����..."
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
  echo "CMake����ʧ�ܣ��������: $LASTEXITCODE"
  exit $LASTEXITCODE
}

# ִ�й���
# PowerShell�л�ȡ�߼�����������
$processorCount = (Get-CimInstance -ClassName Win32_Processor).NumberOfLogicalProcessors
echo "ʹ�� $processorCount �����������й���..."
& "C:/Qt/6.9.1/mingw_64/bin/mingw32-make.exe" -j $processorCount

if ($LASTEXITCODE -ne 0) {
  echo "����ʧ�ܣ��������: $LASTEXITCODE"
  exit $LASTEXITCODE
}

echo "Build completed successfully!"



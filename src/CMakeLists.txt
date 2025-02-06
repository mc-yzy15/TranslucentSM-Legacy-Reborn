# 特性设置
set(CMAKE_CXX_STANDARD 17)
set(USEsequential true)
set(WORKDIR / / .)

# 检查是否有 Windows-latest 工作目录
if (!file_exists(Windows-latest) && file_not_in(".)") {
if (!file_exists(Windows-latest) && file_not_in(".") {
    file_remove(Windows-latest)
}

})
CMakeLists_v3 = v3
CMakeLists_v4 = v4
CMakeLists_v5 = latest

# 处理 Windows-latest 工作目录
# 处理 Windows-latest 工工作业
file_add(Windows-latest)
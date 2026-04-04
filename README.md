# TranslucentSM-Legacy-Reborn

An independent continuation of [rounk-ctrl/TranslucentSM](https://github.com/rounk-ctrl/TranslucentSM). This fork keeps the original idea alive, but the primary implementation is now `StartTAP` and the primary shipped binary is `StartTAP.dll`.

## Project Status

- Primary implementation: `StartTAP`
- Primary build baseline: `NuGet restore` + `MSBuild`
- Supported target matrix for this branch: Windows 10/11, x64 and x86
- Legacy C/CMake implementation: kept only as a fallback path during transition

## Releases

Download the latest release from the [Releases](https://github.com/mc-yzy15/TranslucentSM-Legacy-Reborn/releases) page.

Release assets now ship `StartTAP.dll` as the default binary, packaged separately for x64 and x86.

## Build From Source

1. Restore NuGet packages:

```powershell
nuget restore start.sln
```

2. Build the supported solution entrypoint:

```powershell
msbuild start.sln /m /p:Configuration=Release /p:Platform=x64
msbuild start.sln /m /p:Configuration=Release /p:Platform=x86
```

3. The default outputs are written to:

```text
build/msbuild/x64/Release/StartTAP.dll
build/msbuild/Win32/Release/StartTAP.dll
```

You can also build the project directly:

```powershell
msbuild src\StartTAP\StartTAP.vcxproj /m /p:Configuration=Release /p:Platform=x64
msbuild src\StartTAP\StartTAP.vcxproj /m /p:Configuration=Release /p:Platform=Win32
```

## Settings

The runtime settings continue to use the existing registry contract under `HKEY_CURRENT_USER\SOFTWARE\TranslucentSM`.

Supported keys in this stabilization line:

- `TintOpacity`
- `TintLuminosityOpacity`
- `HideSearch`
- `HideBorder`
- `HideRecommended`
- `EditButton`

After changing settings, terminate `StartMenuExperienceHost.exe` and relaunch the app if the shell does not refresh automatically.

## Legacy Fallback

The CMake files under `src/` are now explicitly for the legacy C fallback only. `StartTAP` is not supported through CMake.

If you need the fallback build path:

```powershell
.\src\Cmake.ps1
```

This builds only the legacy targets and does not produce `StartTAP.dll`.

## Contributing

Bug reports and pull requests are welcome:

- [Issues](https://github.com/mc-yzy15/TranslucentSM-Legacy-Reborn/issues)
- [Pull Requests](https://github.com/mc-yzy15/TranslucentSM-Legacy-Reborn/pulls)

## License

This project is licensed under the GNU General Public License v3.0. See [LICENSE](LICENSE).

# 中文

这是 [rounk-ctrl/TranslucentSM](https://github.com/rounk-ctrl/TranslucentSM) 的非官方延续分支。当前主实现已经切换到 `StartTAP`，默认发布产物为 `StartTAP.dll`。

## 当前状态

- 主实现：`StartTAP`
- 官方构建链：`NuGet restore` + `MSBuild`
- 当前支持范围：Windows 10/11，x64 和 x86
- 旧版 C/CMake 实现：仅作为过渡期 fallback 保留

## 从源码构建

1. 先恢复 NuGet 包：

```powershell
nuget restore start.sln
```

2. 再使用 MSBuild 构建：

```powershell
msbuild start.sln /m /p:Configuration=Release /p:Platform=x64
msbuild start.sln /m /p:Configuration=Release /p:Platform=x86
```

3. 默认输出目录：

```text
build/msbuild/x64/Release/StartTAP.dll
build/msbuild/Win32/Release/StartTAP.dll
```

也可以直接构建工程文件：

```powershell
msbuild src\StartTAP\StartTAP.vcxproj /m /p:Configuration=Release /p:Platform=x64
msbuild src\StartTAP\StartTAP.vcxproj /m /p:Configuration=Release /p:Platform=Win32
```

## 设置项

运行时设置继续沿用 `HKEY_CURRENT_USER\SOFTWARE\TranslucentSM` 下的既有键名：

- `TintOpacity`
- `TintLuminosityOpacity`
- `HideSearch`
- `HideBorder`
- `HideRecommended`
- `EditButton`

如果修改设置后 shell 没有自动刷新，请结束 `StartMenuExperienceHost.exe` 后重新启动应用。

## Legacy Fallback

`src/` 下的 CMake 现在只负责旧版 C 实现的 fallback 构建，不再承诺构建 `StartTAP`。

```powershell
.\src\Cmake.ps1
```

这条路径只会生成旧版 fallback 产物，不会生成 `StartTAP.dll`。

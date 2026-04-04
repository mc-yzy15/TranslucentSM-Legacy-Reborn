# TranslucentSM-Legacy-Reborn

A lightweight utility that makes the Windows 11 Start Menu translucent/transparent.

[简体中文](#简体中文) | [English](#english)

## English

### Features
- Start Menu transparency adjustment UI.
- Runtime language switch (`zh-CN` / `en-US`) with persistent config.
- Built-in update check (prefers installer asset `Setup.exe`).
- CLI install/uninstall entrypoints.

### Installation (One-Click Setup)
1. Download `TranslucentSM-Setup-*.exe` from [Releases](https://github.com/mc-yzy15/TranslucentSM-Legacy-Reborn/releases).
2. Double-click and follow the installer.
3. Optional: silent install for automation:
   - `TranslucentSM-Setup-*.exe /VERYSILENT`

### CLI
- `--help` Show help.
- `--install <path>` Install app to target path.
- `--uninstall` Uninstall app.
- `--lang <zh-CN|en-US>` Force UI/CLI language for this run.

### Settings & Registry
- Settings are persisted in `QSettings`.
- Runtime install info / fallback settings:
  - `HKEY_CURRENT_USER\Software\TranslucentSM`

### Build
- CMake + Qt6 (`Widgets/Core/Gui/Network/LinguistTools`)
- Translations are built from:
  - `src/translations/translucentsm_zh_CN.ts`
  - `src/translations/translucentsm_en_US.ts`

### License
GNU GPL v3.0. See [LICENSE](LICENSE).

---

## 简体中文

### 功能
- 开始菜单透明度图形化调节。
- 运行时语言切换（`zh-CN` / `en-US`）并持久化保存。
- 内置更新检查（优先下载 `Setup.exe` 安装器资源）。
- 提供命令行安装/卸载入口。

### 安装（一步安装）
1. 在 [Releases](https://github.com/mc-yzy15/TranslucentSM-Legacy-Reborn/releases) 下载 `TranslucentSM-Setup-*.exe`。
2. 双击运行并按向导完成安装。
3. 自动化场景可使用静默安装：
   - `TranslucentSM-Setup-*.exe /VERYSILENT`

### 命令行
- `--help` 显示帮助。
- `--install <path>` 安装到指定目录。
- `--uninstall` 卸载应用。
- `--lang <zh-CN|en-US>` 本次运行强制指定语言。

### 设置与注册表
- 设置通过 `QSettings` 持久化。
- 安装信息/回退配置位于：
  - `HKEY_CURRENT_USER\Software\TranslucentSM`

### 构建
- CMake + Qt6（`Widgets/Core/Gui/Network/LinguistTools`）
- 翻译源文件：
  - `src/translations/translucentsm_zh_CN.ts`
  - `src/translations/translucentsm_en_US.ts`

### 许可证
GNU GPL v3.0，详见 [LICENSE](LICENSE)。

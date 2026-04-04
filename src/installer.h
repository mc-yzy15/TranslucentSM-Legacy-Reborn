#ifndef INSTALLER_H
#define INSTALLER_H

#include <QString>

// 安装TranslucentSM到指定路径
int installTranslucentSM(const QString &installPath);

// 卸载TranslucentSM
int uninstallTranslucentSM();

// 创建Windows快捷方式
bool createShortcut(const QString &targetPath, const QString &shortcutPath);

#endif // INSTALLER_H
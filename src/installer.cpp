#include "installer.h"
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QDebug>
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>

// 安装TranslucentSM到指定路径
int installTranslucentSM(const QString &installPath) {
    QDir installDir(installPath);
    if (!installDir.exists() && !installDir.mkpath(installPath)) {
        qCritical() << "无法创建安装目录: " << installPath;
        return 1;
    }

    // 获取源文件路径
    QString exeSource = QCoreApplication::applicationDirPath() + "/TranslucentSM.exe";
    QString dllSource = QCoreApplication::applicationDirPath() + "/TranslucentSM.dll";
    
    // 目标文件路径
    QString exeDest = installPath + "/TranslucentSM.exe";
    QString dllDest = installPath + "/TranslucentSM.dll";

    // 复制文件
    QFile exeFile(exeSource);
    if (!exeFile.copy(exeDest)) {
        qCritical() << "无法复制主程序文件: " << exeFile.errorString();
        return 1;
    }

    QFile dllFile(dllSource);
    if (!dllFile.copy(dllDest)) {
        qCritical() << "无法复制DLL文件: " << dllFile.errorString();
        QFile::remove(exeDest); // 回滚
        return 1;
    }

    // 保存安装路径到注册表
    QSettings settings("HKEY_CURRENT_USER\\Software\\TranslucentSM", QSettings::NativeFormat);
    settings.setValue("InstallPath", installPath);

    // 创建启动项
    QSettings runSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    runSettings.setValue("TranslucentSM", QDir::toNativeSeparators(exeDest));

    // 创建桌面快捷方式
    QString desktopPath = QDir::homePath() + "/Desktop/TranslucentSM.lnk";
    if (!createShortcut(exeDest, desktopPath)) {
        qWarning() << "创建快捷方式失败";
    }

    // 输出安装进度
    qDebug() << "Progress: 100";
    return 0;
}

// 卸载TranslucentSM
int uninstallTranslucentSM() {
    // 从注册表获取安装路径
    QSettings settings("HKEY_CURRENT_USER\\Software\\TranslucentSM", QSettings::NativeFormat);
    QString installPath = settings.value("InstallPath").toString();

    if (installPath.isEmpty() || !QDir(installPath).exists()) {
        qCritical() << "未找到安装路径";
        return 1;
    }

    // 删除文件
    QString exePath = installPath + "/TranslucentSM.exe";
    QString dllPath = installPath + "/TranslucentSM.dll";

    QFile::remove(exePath);
    QFile::remove(dllPath);

    // 删除注册表项
    QSettings runSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    runSettings.remove("TranslucentSM");

    QSettings appSettings("HKEY_CURRENT_USER\\Software\\TranslucentSM", QSettings::NativeFormat);
    appSettings.clear();

    // 删除快捷方式
    QString desktopShortcut = QDir::homePath() + "/Desktop/TranslucentSM.lnk";
    QFile::remove(desktopShortcut);

    // 删除安装目录
    QDir(installPath).rmdir(installPath);

    // 输出卸载进度
    qDebug() << "Progress: 100";
    return 0;
}

// 创建Windows快捷方式
bool createShortcut(const QString &targetPath, const QString &shortcutPath) {
    HRESULT hres;
    IShellLink* psl;

    // 初始化COM库
    hres = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hres)) {
        return false;
    }

    // 创建IShellLink实例
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;

        // 设置快捷方式目标路径
        psl->SetPath(QDir::toNativeSeparators(targetPath).toStdWString().c_str());

        // 设置快捷方式工作目录
        QString workingDir = QFileInfo(targetPath).absolutePath();
        psl->SetWorkingDirectory(QDir::toNativeSeparators(workingDir).toStdWString().c_str());

        // 设置快捷方式描述
        psl->SetDescription(L"TranslucentSM 透明度设置工具");

        // 查询IPersistFile接口
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);

        if (SUCCEEDED(hres)) {
            WCHAR wsz[MAX_PATH];

            // 将QString转换为宽字符串
            QDir::toNativeSeparators(shortcutPath).toWCharArray(wsz);
            wsz[QDir::toNativeSeparators(shortcutPath).length()] = L'\0';

            // 保存快捷方式
            hres = ppf->Save(wsz, TRUE);
            ppf->Release();
        }
        psl->Release();
    }

    CoUninitialize();
    return SUCCEEDED(hres);
}
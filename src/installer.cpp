#include "installer.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>

#include <objbase.h>
#include <shlobj.h>
#include <windows.h>

namespace {
constexpr const char* kRegistryRoot = "HKEY_CURRENT_USER\\Software\\TranslucentSM";
constexpr const char* kRunRegistry = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";

bool copyWithOverwrite(const QString& sourcePath, const QString& destinationPath, QString* errorMessage) {
    if (!QFileInfo::exists(sourcePath)) {
        if (errorMessage) {
            *errorMessage = QObject::tr("Source file does not exist: %1").arg(sourcePath);
        }
        return false;
    }

    if (QFileInfo::exists(destinationPath) && !QFile::remove(destinationPath)) {
        if (errorMessage) {
            *errorMessage = QObject::tr("Failed to remove existing file: %1").arg(destinationPath);
        }
        return false;
    }

    if (!QFile::copy(sourcePath, destinationPath)) {
        if (errorMessage) {
            *errorMessage = QObject::tr("Failed to copy %1 to %2").arg(sourcePath, destinationPath);
        }
        return false;
    }
    return true;
}
}

int installTranslucentSM(const QString& installPath) {
    QDir installDir(installPath);
    if (!installDir.exists() && !installDir.mkpath(installPath)) {
        qCritical().noquote() << QObject::tr("Cannot create install directory: %1").arg(installPath);
        return 1;
    }

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString exeSource = QDir(appDir).filePath("TranslucentSM.exe");
    const QString dllSource = QDir(appDir).filePath("TranslucentSM.dll");
    const QString exeDest = QDir(installPath).filePath("TranslucentSM.exe");
    const QString dllDest = QDir(installPath).filePath("TranslucentSM.dll");

    QString errorMessage;
    if (!copyWithOverwrite(exeSource, exeDest, &errorMessage)) {
        qCritical().noquote() << errorMessage;
        return 1;
    }
    if (!copyWithOverwrite(dllSource, dllDest, &errorMessage)) {
        qCritical().noquote() << errorMessage;
        QFile::remove(exeDest);
        return 1;
    }

    QSettings settings(kRegistryRoot, QSettings::NativeFormat);
    settings.setValue("InstallPath", QDir::toNativeSeparators(installPath));

    QSettings runSettings(kRunRegistry, QSettings::NativeFormat);
    runSettings.setValue("TranslucentSM", QDir::toNativeSeparators(exeDest));

    const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    const QString shortcutPath = QDir(desktopPath).filePath("TranslucentSM.lnk");
    if (!createShortcut(exeDest, shortcutPath)) {
        qWarning().noquote() << QObject::tr("Failed to create desktop shortcut");
    }

    qDebug() << "Progress: 100";
    return 0;
}

int uninstallTranslucentSM() {
    QSettings settings(kRegistryRoot, QSettings::NativeFormat);
    const QString installPath = settings.value("InstallPath").toString();
    if (installPath.isEmpty()) {
        qCritical().noquote() << QObject::tr("Install path not found");
        return 1;
    }

    const QString exePath = QDir(installPath).filePath("TranslucentSM.exe");
    const QString dllPath = QDir(installPath).filePath("TranslucentSM.dll");
    QFile::remove(exePath);
    QFile::remove(dllPath);

    QSettings runSettings(kRunRegistry, QSettings::NativeFormat);
    runSettings.remove("TranslucentSM");

    const QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    const QString desktopShortcut = QDir(desktopPath).filePath("TranslucentSM.lnk");
    QFile::remove(desktopShortcut);

    QDir installDir(installPath);
    if (installDir.exists()) {
        installDir.removeRecursively();
    }

    settings.clear();
    qDebug() << "Progress: 100";
    return 0;
}

bool createShortcut(const QString& targetPath, const QString& shortcutPath) {
    HRESULT result = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(result)) {
        return false;
    }

    IShellLink* shellLink = nullptr;
    result = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLink,
                              reinterpret_cast<void**>(&shellLink));
    if (FAILED(result) || shellLink == nullptr) {
        CoUninitialize();
        return false;
    }

    shellLink->SetPath(QDir::toNativeSeparators(targetPath).toStdWString().c_str());
    shellLink->SetWorkingDirectory(QFileInfo(targetPath).absolutePath().toStdWString().c_str());
    shellLink->SetDescription(QObject::tr("TranslucentSM Configuration Tool").toStdWString().c_str());

    IPersistFile* persistFile = nullptr;
    result = shellLink->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&persistFile));
    bool success = false;
    if (SUCCEEDED(result) && persistFile != nullptr) {
        WCHAR wsz[MAX_PATH];
        const QString nativeShortcutPath = QDir::toNativeSeparators(shortcutPath);
        nativeShortcutPath.toWCharArray(wsz);
        wsz[nativeShortcutPath.length()] = L'\0';
        success = SUCCEEDED(persistFile->Save(wsz, TRUE));
        persistFile->Release();
    }

    shellLink->Release();
    CoUninitialize();
    return success;
}

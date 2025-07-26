#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    // 处理命令行参数
    if (argc > 1) {
        QString command = argv[1];
        
        if (command == "--install" && argc >= 3) {
            QString installPath = argv[2];
            return installTranslucentSM(installPath);
        } else if (command == "--uninstall") {
            return uninstallTranslucentSM();
        } else if (command == "--help") {
            qDebug() << "TranslucentSM 命令行选项:\n";
            qDebug() << "  --install <path>   安装应用到指定路径\n";
            qDebug() << "  --uninstall        卸载应用\n";
            qDebug() << "  --help             显示帮助信息\n";
            return 0;
        }
    }

    // 启动GUI界面
    QApplication app(argc, argv);
    app.setApplicationName("TranslucentSM");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Yzy15");

    MainWindow mainWindow;
    mainWindow.setWindowTitle("TranslucentSM 配置工具");
    mainWindow.show();

    return app.exec();
}

// 安装TranslucentSM
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
    if (!QFile::copy(exeSource, exeDest)) {
        qCritical() << "无法复制主程序文件: " << QFile::errorString();
        return 1;
    }

    if (!QFile::copy(dllSource, dllDest)) {
        qCritical() << "无法复制DLL文件: " << QFile::errorString();
        QFile::remove(exeDest); // 回滚
        return 1;
    }

    // 创建注册表项
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    settings.setValue("TranslucentSM", QDir::toNativeSeparators(exeDest));

    // 创建快捷方式
    createShortcut(exeDest, QDir::homePath() + "/Desktop/TranslucentSM.lnk");

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

    // 删除快捷方式
    QFile::remove(QDir::homePath() + "/Desktop/TranslucentSM.lnk");

    // 删除安装目录
    QDir(installPath).rmdir(installPath);

    // 输出卸载进度
    qDebug() << "Progress: 100";
    return 0;
}

// 创建快捷方式
bool createShortcut(const QString &targetPath, const QString &shortcutPath) {
    // 实际应用中应实现创建快捷方式的逻辑
    Q_UNUSED(targetPath);
    Q_UNUSED(shortcutPath);
    return true;
}
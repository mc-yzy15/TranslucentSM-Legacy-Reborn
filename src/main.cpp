#include "MainWindow.h"
#include <QApplication>
#include "installer.h"

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
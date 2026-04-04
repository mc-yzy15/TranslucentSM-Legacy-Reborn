#include "MainWindow.h"
#include "installer.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

namespace {
constexpr const char* kConfigOrg = "TranslucentSM";
constexpr const char* kConfigApp = "ConfigTool";

QString normalizeLanguageCode(const QString& languageCode) {
    const QString normalized = languageCode.trimmed().replace('_', '-').toLower();
    if (normalized.startsWith("zh")) {
        return "zh-CN";
    }
    if (normalized.startsWith("en")) {
        return "en-US";
    }
    return "en-US";
}

QString defaultLanguageCode() {
    const QString localeName = QLocale::system().name();
    return localeName.startsWith("zh", Qt::CaseInsensitive) ? "zh-CN" : "en-US";
}

bool loadTranslator(QApplication& app, QTranslator& translator, const QString& languageCode) {
    app.removeTranslator(&translator);
    if (languageCode == "en-US") {
        return true;
    }

    const QString qmBaseName = QString("translucentsm_%1").arg(languageCode).replace("-", "_");
    const QString translationsDir = QDir(QCoreApplication::applicationDirPath()).filePath("translations");
    if (translator.load(qmBaseName, translationsDir)) {
        app.installTranslator(&translator);
        return true;
    }

    qWarning() << "Failed to load translation file:" << qmBaseName << "from" << translationsDir;
    return false;
}

void printHelp() {
    qInfo().noquote() << QObject::tr("TranslucentSM command line options:");
    qInfo().noquote() << QObject::tr("  --install <path>   Install app to target path");
    qInfo().noquote() << QObject::tr("  --uninstall        Uninstall app");
    qInfo().noquote() << QObject::tr("  --lang <code>      Force language: zh-CN or en-US");
    qInfo().noquote() << QObject::tr("  --help             Show help");
}
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("TranslucentSM");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Yzy15");

    QSettings settings(kConfigOrg, kConfigApp);

    QString command;
    QString installPath;
    QString forcedLanguageCode;
    const QStringList args = QCoreApplication::arguments();
    for (int i = 1; i < args.size(); ++i) {
        const QString arg = args[i];
        if (arg == "--install") {
            command = "install";
            if (i + 1 < args.size()) {
                installPath = args[++i];
            }
        } else if (arg == "--uninstall") {
            command = "uninstall";
        } else if (arg == "--help" || arg == "-h") {
            command = "help";
        } else if (arg == "--lang") {
            if (i + 1 < args.size()) {
                forcedLanguageCode = args[++i];
            }
        } else {
            qWarning() << QObject::tr("Unknown argument:") << arg;
            command = "help";
            break;
        }
    }

    const QString selectedLanguageCode = normalizeLanguageCode(
        forcedLanguageCode.isEmpty()
            ? settings.value("UI/Language", defaultLanguageCode()).toString()
            : forcedLanguageCode);

    settings.setValue("UI/Language", selectedLanguageCode);

    QTranslator translator;
    loadTranslator(app, translator, selectedLanguageCode);

    if (command == "help") {
        printHelp();
        return 0;
    }
    if (command == "install") {
        if (installPath.trimmed().isEmpty()) {
            qCritical() << QObject::tr("Missing install path. Usage: --install <path>");
            return 1;
        }
        return installTranslucentSM(installPath);
    }
    if (command == "uninstall") {
        return uninstallTranslucentSM();
    }

    MainWindow mainWindow;
    mainWindow.applyLanguage(selectedLanguageCode);
    QObject::connect(&mainWindow, &MainWindow::languageChangeRequested, &app,
                     [&](const QString& languageCode) {
        const QString normalized = normalizeLanguageCode(languageCode);
        settings.setValue("UI/Language", normalized);
        loadTranslator(app, translator, normalized);
        mainWindow.applyLanguage(normalized);
    });
    mainWindow.show();

    return app.exec();
}

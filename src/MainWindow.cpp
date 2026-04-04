#include "MainWindow.h"
#include "TranslucentSM.h"
#include "installer.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QMouseEvent>
#include <QNetworkRequest>
#include <QPixmap>
#include <QSignalBlocker>
#include <QUrl>

namespace {
constexpr const char* kConfigOrg = "TranslucentSM";
constexpr const char* kConfigApp = "ConfigTool";
constexpr const char* kRegistryRoot = "HKEY_CURRENT_USER\\Software\\TranslucentSM";
}

TitleBar::TitleBar(QWidget* parent)
    : QWidget(parent),
      titleLabel(nullptr),
      minimizeBtn(nullptr),
      closeBtn(nullptr),
      m_moving(false) {
    setFixedHeight(30);
    setStyleSheet("background-color: #2d2d2d;");

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 5, 0);
    layout->setSpacing(0);

    titleLabel = new QLabel(this);
    titleLabel->setStyleSheet("color: #ffffff; font-weight: bold;");
    layout->addWidget(titleLabel, 1);

    minimizeBtn = new QPushButton(this);
    minimizeBtn->setFixedSize(25, 25);
    minimizeBtn->setStyleSheet(
        "QPushButton { background-color: transparent; color: white; border: none; }"
        "QPushButton:hover { background-color: #4a4a4a; border-radius: 3px; }");
    connect(minimizeBtn, &QPushButton::clicked, this, &TitleBar::minimizeWindow);
    layout->addWidget(minimizeBtn);

    closeBtn = new QPushButton(this);
    closeBtn->setFixedSize(25, 25);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: transparent; color: white; border: none; }"
        "QPushButton:hover { background-color: #e81123; border-radius: 3px; }");
    connect(closeBtn, &QPushButton::clicked, this, &TitleBar::closeWindow);
    layout->addWidget(closeBtn);

    setLayout(layout);
    retranslateUi();
}

void TitleBar::retranslateUi() {
    titleLabel->setText(tr("TranslucentSM Configuration Tool"));
    minimizeBtn->setText("-");
    closeBtn->setText(QString::fromUtf8("×"));
}

void TitleBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_startPos = event->globalPosition().toPoint();
        m_moving = true;
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent* event) {
    if (m_moving && (event->buttons() & Qt::LeftButton)) {
        QWidget* parent = parentWidget();
        if (parent) {
            const QPoint delta = event->globalPosition().toPoint() - m_startPos;
            parent->move(parent->pos() + delta);
            m_startPos = event->globalPosition().toPoint();
        }
    } else {
        m_moving = false;
    }
    QWidget::mouseMoveEvent(event);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      updateProgressBar(nullptr),
      installProcess(new QProcess(this)),
      networkManager(new QNetworkAccessManager(this)),
      currentVersion(QCoreApplication::applicationVersion().isEmpty()
                         ? QStringLiteral("1.0.0")
                         : QCoreApplication::applicationVersion()),
      currentLanguageCode("en-US"),
      selectedUpdateAssetName(),
      mainCentralWidget(nullptr),
      titleBar(nullptr),
      mainTabWidget(nullptr),
      progressBar(nullptr),
      statusLabel(nullptr),
      installButton(nullptr),
      uninstallButton(nullptr),
      applyButton(nullptr),
      browseButton(nullptr),
      checkUpdateButton(nullptr),
      installPathEdit(nullptr),
      transparencySlider(nullptr),
      transparencyValueLabel(nullptr),
      themeComboBox(nullptr),
      languageComboBox(nullptr),
      installTabWidget(nullptr),
      settingsTabWidget(nullptr),
      aboutTabWidget(nullptr),
      pathLabel(nullptr),
      infoLabel(nullptr),
      transparencyLabel(nullptr),
      themeLabel(nullptr),
      languageLabel(nullptr),
      aboutNameLabel(nullptr),
      aboutVersionLabel(nullptr),
      aboutAuthorLabel(nullptr),
      aboutGithubLabel(nullptr),
      aboutCopyrightLabel(nullptr) {
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(600, 450);

    applyModernStyle();
    setupUI();
    loadPersistedSettings();
    retranslateUi();
    updateUIState();

    connect(installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onInstallationFinished);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI() {
    mainCentralWidget = new QWidget(this);
    mainCentralWidget->setStyleSheet("background-color: #1e1e1e; border-radius: 8px;");
    setCentralWidget(mainCentralWidget);

    QVBoxLayout* layout = new QVBoxLayout(mainCentralWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    titleBar = new TitleBar(this);
    connect(titleBar, &TitleBar::minimizeWindow, this, &QWidget::showMinimized);
    connect(titleBar, &TitleBar::closeWindow, this, &QWidget::close);
    layout->addWidget(titleBar);

    mainTabWidget = new QTabWidget();
    layout->addWidget(mainTabWidget);

    createInstallationTab();
    createSettingsTab();
    createAboutTab();

    QWidget* statusBarWidget = new QWidget();
    statusBarWidget->setFixedHeight(25);
    statusBarWidget->setStyleSheet("background-color: #252526; color: #b0b0b0;");
    QHBoxLayout* statusLayout = new QHBoxLayout(statusBarWidget);
    statusLayout->setContentsMargins(10, 0, 10, 0);

    statusLabel = new QLabel();
    statusLayout->addWidget(statusLabel, 1);

    progressBar = new QProgressBar();
    progressBar->setFixedHeight(15);
    progressBar->setValue(0);
    progressBar->setVisible(false);
    statusLayout->addWidget(progressBar, 2);
    layout->addWidget(statusBarWidget);
}

void MainWindow::applyModernStyle() {
    qApp->setStyle(QStyleFactory::create("Fusion"));
    qApp->setStyleSheet(
        "QPushButton { padding: 8px 16px; border-radius: 6px; border: none; background-color: #0078d7; color: white; }"
        "QPushButton:hover { background-color: #005a9e; }"
        "QLineEdit, QComboBox { border: 1px solid #ced4da; border-radius: 6px; padding: 6px; background-color: white; color: #212529; }"
        "QTabWidget::pane { border: none; background-color: #1e1e1e; }"
        "QTabBar::tab { height: 30px; width: 130px; background-color: #2d2d2d; color: #b0b0b0; }"
        "QTabBar::tab:selected { background-color: #3d3d3d; color: white; }");
}

void MainWindow::createInstallationTab() {
    installTabWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(installTabWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLabel = new QLabel(installTabWidget);
    installPathEdit = new QLineEdit(installTabWidget);
    installPathEdit->setText(QDir::toNativeSeparators(QDir::homePath() + "/AppData/Local/TranslucentSM"));
    browseButton = new QPushButton(installTabWidget);
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseClicked);
    pathLayout->addWidget(pathLabel, 0);
    pathLayout->addWidget(installPathEdit, 1);
    pathLayout->addWidget(browseButton, 0);
    layout->addLayout(pathLayout);

    infoLabel = new QLabel(installTabWidget);
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #b0b0b0;");
    layout->addWidget(infoLabel);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);
    buttonLayout->setAlignment(Qt::AlignCenter);

    installButton = new QPushButton(installTabWidget);
    installButton->setFixedSize(120, 35);
    connect(installButton, &QPushButton::clicked, this, &MainWindow::onInstallClicked);

    uninstallButton = new QPushButton(installTabWidget);
    uninstallButton->setFixedSize(120, 35);
    connect(uninstallButton, &QPushButton::clicked, this, &MainWindow::onUninstallClicked);

    buttonLayout->addWidget(installButton);
    buttonLayout->addWidget(uninstallButton);
    layout->addLayout(buttonLayout);
    layout->addStretch(1);

    mainTabWidget->addTab(installTabWidget, QString());
}

void MainWindow::createSettingsTab() {
    settingsTabWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(settingsTabWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    QWidget* transparencyWidget = new QWidget(settingsTabWidget);
    QVBoxLayout* transparencyLayout = new QVBoxLayout(transparencyWidget);
    transparencyLabel = new QLabel(transparencyWidget);
    transparencyLabel->setStyleSheet("font-weight: bold;");
    transparencyLayout->addWidget(transparencyLabel);

    QHBoxLayout* sliderLayout = new QHBoxLayout();
    transparencySlider = new QSlider(Qt::Horizontal, transparencyWidget);
    transparencySlider->setRange(0, 100);
    transparencyValueLabel = new QLabel("50%", transparencyWidget);
    connect(transparencySlider, &QSlider::valueChanged, this, [this](int value) {
        transparencyValueLabel->setText(QString("%1%").arg(value));
    });
    sliderLayout->addWidget(transparencySlider);
    sliderLayout->addWidget(transparencyValueLabel);
    transparencyLayout->addLayout(sliderLayout);
    layout->addWidget(transparencyWidget);

    QWidget* themeWidget = new QWidget(settingsTabWidget);
    QVBoxLayout* themeLayout = new QVBoxLayout(themeWidget);
    themeLabel = new QLabel(themeWidget);
    themeLabel->setStyleSheet("font-weight: bold;");
    themeLayout->addWidget(themeLabel);
    themeComboBox = new QComboBox(themeWidget);
    themeLayout->addWidget(themeComboBox);
    layout->addWidget(themeWidget);

    QWidget* languageWidget = new QWidget(settingsTabWidget);
    QVBoxLayout* languageLayout = new QVBoxLayout(languageWidget);
    languageLabel = new QLabel(languageWidget);
    languageLabel->setStyleSheet("font-weight: bold;");
    languageLayout->addWidget(languageLabel);
    languageComboBox = new QComboBox(languageWidget);
    languageLayout->addWidget(languageComboBox);
    layout->addWidget(languageWidget);
    connect(languageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLanguageChanged);

    layout->addStretch(1);

    applyButton = new QPushButton(settingsTabWidget);
    applyButton->setFixedSize(140, 35);
    connect(applyButton, &QPushButton::clicked, this, &MainWindow::onApplySettingsClicked);
    layout->addWidget(applyButton, 0, Qt::AlignCenter);

    checkUpdateButton = new QPushButton(settingsTabWidget);
    checkUpdateButton->setFixedSize(140, 35);
    connect(checkUpdateButton, &QPushButton::clicked, this, &MainWindow::checkUpdateClicked);
    layout->addWidget(checkUpdateButton, 0, Qt::AlignCenter);

    updateProgressBar = new QProgressBar(settingsTabWidget);
    updateProgressBar->setVisible(false);
    layout->addWidget(updateProgressBar);

    mainTabWidget->addTab(settingsTabWidget, QString());
}

void MainWindow::createAboutTab() {
    aboutTabWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(aboutTabWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);
    layout->setAlignment(Qt::AlignCenter);

    QLabel* iconLabel = new QLabel(aboutTabWidget);
    iconLabel->setPixmap(QPixmap(32, 32));
    iconLabel->setFixedSize(64, 64);
    iconLabel->setStyleSheet("background-color: #007acc; border-radius: 8px;");
    layout->addWidget(iconLabel, 0, Qt::AlignCenter);

    aboutNameLabel = new QLabel(aboutTabWidget);
    aboutNameLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    layout->addWidget(aboutNameLabel, 0, Qt::AlignCenter);

    aboutVersionLabel = new QLabel(aboutTabWidget);
    aboutVersionLabel->setStyleSheet("color: #b0b0b0;");
    layout->addWidget(aboutVersionLabel, 0, Qt::AlignCenter);

    QFrame* line = new QFrame(aboutTabWidget);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #555555;");
    layout->addWidget(line);

    aboutAuthorLabel = new QLabel(aboutTabWidget);
    layout->addWidget(aboutAuthorLabel, 0, Qt::AlignCenter);

    aboutGithubLabel = new QLabel(aboutTabWidget);
    aboutGithubLabel->setOpenExternalLinks(true);
    layout->addWidget(aboutGithubLabel, 0, Qt::AlignCenter);

    aboutCopyrightLabel = new QLabel(aboutTabWidget);
    aboutCopyrightLabel->setStyleSheet("color: #b0b0b0; font-size: 10px;");
    layout->addWidget(aboutCopyrightLabel, 0, Qt::AlignCenter);

    mainTabWidget->addTab(aboutTabWidget, QString());
}

void MainWindow::loadPersistedSettings() {
    QSettings settings(kConfigOrg, kConfigApp);
    const int transparencyValue = settings.value("transparency", 50).toInt();
    transparencySlider->setValue(qBound(0, transparencyValue, 100));

    const int themeIndex = settings.value("theme", 0).toInt();
    themeComboBox->setCurrentIndex(qBound(0, themeIndex, 2));

    currentLanguageCode = normalizedLanguageCode(
        settings.value("UI/Language", detectDefaultLanguage()).toString());
    setupLanguageSelector();

    const QString savedInstallPath = settings.value("InstallPath").toString();
    if (!savedInstallPath.trimmed().isEmpty()) {
        installPathEdit->setText(QDir::toNativeSeparators(savedInstallPath));
    }
}

QString MainWindow::detectDefaultLanguage() const {
    const QString localeName = QLocale::system().name();
    return localeName.startsWith("zh", Qt::CaseInsensitive) ? "zh-CN" : "en-US";
}

QString MainWindow::normalizedLanguageCode(const QString& languageCode) const {
    const QString normalized = languageCode.trimmed().replace('_', '-').toLower();
    if (normalized.startsWith("zh")) {
        return "zh-CN";
    }
    if (normalized.startsWith("en")) {
        return "en-US";
    }
    return "en-US";
}

void MainWindow::setupLanguageSelector() {
    if (!languageComboBox) {
        return;
    }

    const QString normalizedCode = normalizedLanguageCode(
        currentLanguageCode.isEmpty() ? detectDefaultLanguage() : currentLanguageCode);
    QSignalBlocker blocker(languageComboBox);

    languageComboBox->clear();
    languageComboBox->addItem(tr("简体中文 (zh-CN)"), "zh-CN");
    languageComboBox->addItem(tr("English (en-US)"), "en-US");

    int index = languageComboBox->findData(normalizedCode);
    if (index < 0) {
        index = languageComboBox->findData("en-US");
    }
    languageComboBox->setCurrentIndex(index);
}

void MainWindow::retranslateUi() {
    setWindowTitle(tr("TranslucentSM Configuration Tool"));
    titleBar->retranslateUi();

    mainTabWidget->setTabText(mainTabWidget->indexOf(installTabWidget), tr("Install/Uninstall"));
    mainTabWidget->setTabText(mainTabWidget->indexOf(settingsTabWidget), tr("Settings"));
    mainTabWidget->setTabText(mainTabWidget->indexOf(aboutTabWidget), tr("About"));

    pathLabel->setText(tr("Install Path:"));
    browseButton->setText(tr("Browse..."));
    infoLabel->setText(
        tr("TranslucentSM adds transparency effects to the Windows 11 Start Menu.\n"
           "Requires Windows 11 build 22000 or later."));
    installButton->setText(tr("Install"));
    uninstallButton->setText(tr("Uninstall"));

    transparencyLabel->setText(tr("Transparency:"));
    themeLabel->setText(tr("Theme:"));
    const int themeIndex = themeComboBox->currentIndex();
    themeComboBox->clear();
    themeComboBox->addItem(tr("Default"));
    themeComboBox->addItem(tr("Dark"));
    themeComboBox->addItem(tr("Light"));
    themeComboBox->setCurrentIndex(qBound(0, themeIndex, 2));

    languageLabel->setText(tr("Language:"));
    setupLanguageSelector();
    applyButton->setText(tr("Apply Settings"));
    checkUpdateButton->setText(tr("Check for Updates"));

    aboutNameLabel->setText("TranslucentSM");
    aboutVersionLabel->setText(tr("Version %1").arg(currentVersion));
    aboutAuthorLabel->setText(tr("Author: Yzy15"));
    aboutGithubLabel->setText(
        tr("<a href='https://github.com/mc-yzy15/TranslucentSM-Legacy-Reborn'>GitHub Repository</a>"));
    aboutCopyrightLabel->setText(tr("© 2026 All Rights Reserved"));

    updateUIState();
}

void MainWindow::applyLanguage(const QString& languageCode) {
    currentLanguageCode = normalizedLanguageCode(languageCode);
    retranslateUi();
}

void MainWindow::onLanguageChanged(int index) {
    if (index < 0) {
        return;
    }

    const QString selectedLanguage = normalizedLanguageCode(languageComboBox->itemData(index).toString());
    if (selectedLanguage == currentLanguageCode) {
        return;
    }

    currentLanguageCode = selectedLanguage;
    QSettings settings(kConfigOrg, kConfigApp);
    settings.setValue("UI/Language", currentLanguageCode);
    emit languageChangeRequested(currentLanguageCode);
}

void MainWindow::onInstallClicked() {
    const QString installPath = getInstallPath();
    QDir installDir(installPath);

    if (!installDir.exists() && !installDir.mkpath(installPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to create install directory."));
        return;
    }

    statusLabel->setText(tr("Installing..."));
    progressBar->setVisible(true);
    progressBar->setValue(0);
    installButton->setEnabled(false);
    uninstallButton->setEnabled(false);

    const int result = installTranslucentSM(installPath);
    if (result == 0) {
        QSettings settings(kConfigOrg, kConfigApp);
        settings.setValue("InstallPath", installPath);

        progressBar->setValue(100);
        statusLabel->setText(tr("Install completed"));
        QMessageBox::information(this, tr("Success"), tr("Installation finished successfully."));
        updateUIState();
    } else {
        statusLabel->setText(tr("Install failed"));
        QMessageBox::critical(this, tr("Error"), tr("Installation failed. Please check logs for details."));
    }

    progressBar->setVisible(false);
    installButton->setEnabled(true);
    uninstallButton->setEnabled(true);
}

void MainWindow::onUninstallClicked() {
    const QMessageBox::StandardButton confirm = QMessageBox::question(
        this,
        tr("Confirm"),
        tr("Are you sure you want to uninstall TranslucentSM?"),
        QMessageBox::Yes | QMessageBox::No);
    if (confirm != QMessageBox::Yes) {
        return;
    }

    statusLabel->setText(tr("Uninstalling..."));
    progressBar->setVisible(true);
    progressBar->setValue(0);
    installButton->setEnabled(false);
    uninstallButton->setEnabled(false);

    const int result = uninstallTranslucentSM();
    if (result == 0) {
        progressBar->setValue(100);
        statusLabel->setText(tr("Uninstall completed"));
        QMessageBox::information(this, tr("Success"), tr("Uninstallation finished successfully."));
        updateUIState();
    } else {
        statusLabel->setText(tr("Uninstall failed"));
        QMessageBox::critical(this, tr("Error"), tr("Uninstallation failed. Please check logs for details."));
    }

    progressBar->setVisible(false);
    installButton->setEnabled(true);
    uninstallButton->setEnabled(true);
}

void MainWindow::onApplySettingsClicked() {
    QSettings settings(kConfigOrg, kConfigApp);
    const int transparencyValue = transparencySlider->value();
    settings.setValue("transparency", transparencyValue);
    settings.setValue("theme", themeComboBox->currentIndex());
    settings.setValue("UI/Language", currentLanguageCode);

    TranslucentSM translucentSM;
    translucentSM.applyTransparencySettings("StartMenuExperienceHost.exe", transparencyValue);

    statusLabel->setText(tr("Settings applied"));
    QMessageBox::information(this, tr("Success"), tr("Transparency settings have been applied."));
}

void MainWindow::onBrowseClicked() {
    const QString path = QFileDialog::getExistingDirectory(
        this,
        tr("Select Install Directory"),
        installPathEdit->text());
    if (!path.isEmpty()) {
        installPathEdit->setText(QDir::toNativeSeparators(path));
    }
}

void MainWindow::updateProgress(int value) {
    progressBar->setValue(value);
    statusLabel->setText(tr("Processing... %1%").arg(value));
}

void MainWindow::onInstallationFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        statusLabel->setText(tr("Operation completed"));
        QMessageBox::information(this, tr("Success"), tr("Operation finished successfully."));
        updateUIState();
    } else {
        statusLabel->setText(tr("Operation failed"));
        QMessageBox::critical(this, tr("Error"), tr("Operation failed. Please check logs for details."));
    }
    progressBar->setVisible(false);
    installButton->setEnabled(true);
    uninstallButton->setEnabled(true);
}

bool MainWindow::checkInstallationStatus() {
    QSettings registrySettings(kRegistryRoot, QSettings::NativeFormat);
    const QString installPath = registrySettings.value("InstallPath").toString();
    if (installPath.trimmed().isEmpty()) {
        return false;
    }

    const QString exePath = QDir(installPath).filePath("TranslucentSM.exe");
    const QString dllPath = QDir(installPath).filePath("TranslucentSM.dll");
    const bool installed = QFileInfo::exists(exePath) && QFileInfo::exists(dllPath);
    if (installed) {
        installPathEdit->setText(QDir::toNativeSeparators(installPath));
    }
    return installed;
}

void MainWindow::updateUIState() {
    const bool isInstalled = checkInstallationStatus();
    installButton->setEnabled(!isInstalled);
    uninstallButton->setEnabled(isInstalled);
    applyButton->setEnabled(isInstalled);

    if (isInstalled) {
        statusLabel->setText(tr("Installed: TranslucentSM"));
    } else {
        statusLabel->setText(tr("Not installed"));
    }
}

QString MainWindow::getInstallPath() {
    return installPathEdit->text().trimmed();
}

void MainWindow::checkUpdateClicked() {
    statusLabel->setText(tr("Checking for updates..."));
    selectedUpdateAssetName.clear();

    QNetworkRequest request(QUrl("https://api.github.com/repos/mc-yzy15/TranslucentSM-Legacy-Reborn/releases/latest"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "TranslucentSM");

    QNetworkReply* reply = networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onUpdateCheckFinished(reply);
    });
}

void MainWindow::onUpdateCheckFinished(QNetworkReply* reply) {
    if (!reply) {
        statusLabel->setText(tr("Update check failed"));
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        statusLabel->setText(tr("Update check failed: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject releaseObject;
    if (doc.isObject()) {
        releaseObject = doc.object();
    } else if (doc.isArray() && !doc.array().isEmpty()) {
        releaseObject = doc.array().first().toObject();
    }

    if (releaseObject.isEmpty()) {
        statusLabel->setText(tr("No release information found"));
        reply->deleteLater();
        return;
    }

    const QString latestVersion = releaseObject.value("tag_name").toString().remove("v", Qt::CaseInsensitive);
    const QJsonArray assets = releaseObject.value("assets").toArray();

    QString downloadUrl;
    for (const QJsonValue& assetValue : assets) {
        const QJsonObject asset = assetValue.toObject();
        const QString name = asset.value("name").toString();
        const QString lower = name.toLower();
        if (lower.contains("setup") && lower.endsWith(".exe")) {
            selectedUpdateAssetName = name;
            downloadUrl = asset.value("browser_download_url").toString();
            break;
        }
    }

    if (downloadUrl.isEmpty()) {
        for (const QJsonValue& assetValue : assets) {
            const QJsonObject asset = assetValue.toObject();
            const QString name = asset.value("name").toString();
            if (name.toLower().endsWith(".exe")) {
                selectedUpdateAssetName = name;
                downloadUrl = asset.value("browser_download_url").toString();
                break;
            }
        }
    }

    if (downloadUrl.isEmpty()) {
        for (const QJsonValue& assetValue : assets) {
            const QJsonObject asset = assetValue.toObject();
            const QString name = asset.value("name").toString();
            if (name.toLower().endsWith(".zip")) {
                selectedUpdateAssetName = name;
                downloadUrl = asset.value("browser_download_url").toString();
                break;
            }
        }
    }

    if (downloadUrl.isEmpty()) {
        statusLabel->setText(tr("No downloadable asset found"));
        reply->deleteLater();
        return;
    }

    if (versionCompare(latestVersion, currentVersion) > 0) {
        const QMessageBox::StandardButton confirm = QMessageBox::question(
            this,
            tr("Update Available"),
            tr("A new version is available: %1\nDo you want to download and install it?").arg(latestVersion),
            QMessageBox::Yes | QMessageBox::No);

        if (confirm == QMessageBox::Yes) {
            statusLabel->setText(tr("Downloading update..."));
            updateProgressBar->setVisible(true);
            updateProgressBar->setValue(0);

            QNetworkReply* downloadReply = networkManager->get(QNetworkRequest(QUrl(downloadUrl)));
            connect(downloadReply, &QNetworkReply::downloadProgress, this, &MainWindow::onDownloadProgress);
            connect(downloadReply, &QNetworkReply::finished, this, &MainWindow::onDownloadFinished);
        } else {
            statusLabel->setText(tr("Update cancelled"));
        }
    } else {
        statusLabel->setText(tr("You are using the latest version"));
    }

    reply->deleteLater();
}

void MainWindow::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        const int percent = static_cast<int>((bytesReceived * 100) / bytesTotal);
        updateProgressBar->setValue(percent);
        statusLabel->setText(tr("Downloading update: %1%").arg(percent));
    }
}

void MainWindow::onDownloadFinished() {
    updateProgressBar->setVisible(false);

    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        statusLabel->setText(tr("Update download failed"));
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        statusLabel->setText(tr("Update download failed: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

    QString assetName = selectedUpdateAssetName.trimmed();
    if (assetName.isEmpty()) {
        assetName = "TranslucentSM-Update-Setup.exe";
    }
    assetName = QFileInfo(assetName).fileName();

    const QString tempPath = QDir::temp().filePath(assetName);
    QFile file(tempPath);
    if (!file.open(QIODevice::WriteOnly)) {
        statusLabel->setText(tr("Cannot save update package"));
        reply->deleteLater();
        return;
    }
    file.write(reply->readAll());
    file.close();

    const bool isInstaller = assetName.toLower().endsWith(".exe");
    bool started = false;
    if (isInstaller) {
        started = QProcess::startDetached(tempPath, QStringList());
    } else {
        started = QDesktopServices::openUrl(QUrl::fromLocalFile(tempPath));
    }

    if (!started) {
        statusLabel->setText(tr("Failed to launch update package"));
    } else if (isInstaller) {
        statusLabel->setText(tr("Installer launched. The app will exit now."));
        qApp->quit();
    } else {
        statusLabel->setText(tr("Update package downloaded"));
    }

    reply->deleteLater();
}

int MainWindow::versionCompare(const QString& version1, const QString& version2) {
    const QStringList v1 = version1.split(".");
    const QStringList v2 = version2.split(".");
    const int maxLen = qMax(v1.size(), v2.size());

    for (int i = 0; i < maxLen; ++i) {
        const int num1 = i < v1.size() ? v1[i].toInt() : 0;
        const int num2 = i < v2.size() ? v2[i].toInt() : 0;
        if (num1 > num2) {
            return 1;
        }
        if (num1 < num2) {
            return -1;
        }
    }

    return 0;
}

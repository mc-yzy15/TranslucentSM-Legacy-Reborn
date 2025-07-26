#include "MainWindow.h"
#include "installer.h"
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QGuiApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>

// 标题栏实现
TitleBar::TitleBar(QWidget *parent) : QWidget(parent), m_moving(false) {
    setFixedHeight(30);
    setStyleSheet("background-color: #2d2d2d;");

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 5, 0);
    layout->setSpacing(0);

    // 标题标签
    QLabel *titleLabel = new QLabel("TranslucentSM 配置工具");
    titleLabel->setStyleSheet("color: #ffffff; font-weight: bold;");
    layout->addWidget(titleLabel, 1);

    // 最小化按钮
    QPushButton *minimizeBtn = new QPushButton("-");
    minimizeBtn->setFixedSize(25, 25);
    minimizeBtn->setStyleSheet(
        "QPushButton { background-color: transparent; color: white; border: none; }"
        "QPushButton:hover { background-color: #4a4a4a; border-radius: 3px; }"
    );
    connect(minimizeBtn, &QPushButton::clicked, this, &TitleBar::minimizeWindow);
    layout->addWidget(minimizeBtn);

    // 关闭按钮
    QPushButton *closeBtn = new QPushButton("×");
    closeBtn->setFixedSize(25, 25);
    closeBtn->setStyleSheet(
        "QPushButton { background-color: transparent; color: white; border: none; }"
        "QPushButton:hover { background-color: #e81123; border-radius: 3px; }"
    );
    connect(closeBtn, &QPushButton::clicked, this, &TitleBar::closeWindow);
    layout->addWidget(closeBtn);

    setLayout(layout);
}

void TitleBar::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_startPos = event->globalPos();
        m_moving = true;
    }
    QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event) {
    if (m_moving && (event->buttons() & Qt::LeftButton)) {
        QWidget *parent = parentWidget();
        if (parent) {
            QPoint delta = event->globalPos() - m_startPos;
            parent->move(parent->pos() + delta);
            m_startPos = event->globalPos();
        }
    }
    QWidget::mouseMoveEvent(event);
}

// 主窗口实现
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), installProcess(new QProcess(this)) {
    // 设置窗口属性
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(600, 450);

    // 应用现代样式
    applyModernStyle();

    // 创建中心部件
    centralWidget = new QWidget(this);
    centralWidget->setStyleSheet("background-color: #1e1e1e; border-radius: 8px;");
    setCentralWidget(centralWidget);

    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 添加自定义标题栏
    TitleBar *titleBar = new TitleBar(this);
    connect(titleBar, &TitleBar::minimizeWindow, this, &QWidget::showMinimized);
    connect(titleBar, &TitleBar::closeWindow, this, &QWidget::close);
    mainLayout->addWidget(titleBar);

    // 创建标签页部件
    mainTabWidget = new QTabWidget();
    mainTabWidget->setStyleSheet(
        "QTabWidget::pane { border: none; background-color: #1e1e1e; }"
        "QTabBar::tab { height: 30px; width: 120px; background-color: #2d2d2d; color: #b0b0b0; border: none; margin-right: 2px; }"
        "QTabBar::tab:selected { background-color: #3d3d3d; color: white; border-top-left-radius: 4px; border-top-right-radius: 4px; }"
        "QTabBar::tab:hover:!selected { background-color: #353535; }"
    );
    mainLayout->addWidget(mainTabWidget);

    // 创建各个标签页
    createInstallationTab();
    createSettingsTab();
    createAboutTab();

    // 状态栏
    QWidget *statusBar = new QWidget();
    statusBar->setFixedHeight(25);
    statusBar->setStyleSheet("background-color: #252526; color: #b0b0b0;");
    QHBoxLayout *statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(10, 0, 10, 0);

    statusLabel = new QLabel("就绪");
    statusLabel->setStyleSheet("color: #b0b0b0;");
    statusLayout->addWidget(statusLabel, 1);

    progressBar = new QProgressBar();
    progressBar->setFixedHeight(15);
    progressBar->setStyleSheet(
        "QProgressBar { border: none; background-color: #3d3d3d; border-radius: 7px; }"
        "QProgressBar::chunk { background-color: #007acc; border-radius: 7px; }"
    );
    progressBar->setValue(0);
    progressBar->setVisible(false);
    statusLayout->addWidget(progressBar, 2);

    mainLayout->addWidget(statusBar);

    // 检查安装状态并更新UI
    updateUIState();

    // 连接进程信号
    connect(installProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::onInstallationFinished);
}

MainWindow::~MainWindow() {
    delete installProcess;
}

void MainWindow::setupUI() {
    // 已在构造函数中实现UI设置
}

void MainWindow::applyModernStyle() {
    // 设置全局样式
    qApp->setStyle(QStyleFactory::create("Fusion"));

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(30, 30, 30));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(45, 45, 45));
    palette.setColor(QPalette::AlternateBase, QColor(55, 55, 55));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(55, 55, 55));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(palette);

    // 设置全局样式表
    qApp->setStyleSheet(
        "QPushButton { padding: 8px 16px; border-radius: 6px; border: none; background-color: #0078d7; color: white; font-weight: 500; }"
        "QPushButton:hover { background-color: #005a9e; }"
        "QPushButton:pressed { background-color: #004b87; }"
        "QPushButton:disabled { background-color: #555555; color: #aaaaaa; }"
        "QSlider::groove:horizontal { background-color: #e0e0e0; height: 6px; border-radius: 3px; }"
        "QSlider::handle:horizontal { background-color: #0078d7; width: 18px; height: 18px; border-radius: 9px; margin: -6px 0; border: none; }"
        "QLineEdit { border: 1px solid #ced4da; border-radius: 6px; padding: 8px; background-color: white; color: #212529; }"
        "QComboBox { padding: 8px; border-radius: 6px; border: 1px solid #ced4da; background-color: white; color: #212529; }"
        "QComboBox::drop-down { border-top-right-radius: 6px; border-bottom-right-radius: 6px; background-color: #e9ecef; }"
        "QLabel { color: #212529; }"
        "QTabWidget::pane { border: 1px solid #e0e0e0; border-radius: 8px; background-color: white; }"
        "QTabBar::tab { background-color: #e9ecef; color: #495057; padding: 8px 16px; border-radius: 8px 8px 0 0; margin-right: 4px; }"
        "QTabBar::tab:selected { background-color: white; color: #0078d7; border-top: 2px solid #0078d7; }"
        "QProgressBar { border-radius: 6px; text-align: center; background-color: #e9ecef; height: 8px; }"
        "QProgressBar::chunk { background-color: #0078d7; border-radius: 4px; }"
        "QCheckBox { spacing: 8px; color: #495057; }"
        "QCheckBox::indicator { width: 18px; height: 18px; border-radius: 4px; border: 2px solid #adb5bd; }"
        "QCheckBox::indicator:checked { background-color: #0078d7; border-color: #0078d7; image: url(:/icons/check.svg); }"
        "QMessageBox { background-color: white; border-radius: 8px; }"
    );
}

void MainWindow::createInstallationTab() {
    QWidget *installWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(installWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    // 安装路径选择
    QHBoxLayout *pathLayout = new QHBoxLayout();
    QLabel *pathLabel = new QLabel("安装路径:");
    installPathEdit = new QLineEdit();
    installPathEdit->setText(QDir::toNativeSeparators(QDir::homePath() + "/AppData/Local/TranslucentSM"));
    QPushButton *browseButton = new QPushButton("浏览...");
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseClicked);

    pathLayout->addWidget(pathLabel, 0);
    pathLayout->addWidget(installPathEdit, 1);
    pathLayout->addWidget(browseButton, 0);
    layout->addLayout(pathLayout);

    // 安装说明
    QLabel *infoLabel = new QLabel(
        "TranslucentSM 将为您的 Windows 11 开始菜单添加透明效果。\n"\n        "安装过程将复制必要的文件并设置自动启动项。");
    infoLabel->setWordWrap(true);
    infoLabel->setStyleSheet("color: #b0b0b0; margin-top: 10px;");
    layout->addWidget(infoLabel);

    // 按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);
    buttonLayout->setAlignment(Qt::AlignCenter);

    installButton = new QPushButton("安装");
    installButton->setFixedSize(120, 35);
    connect(installButton, &QPushButton::clicked, this, &MainWindow::onInstallClicked);

    uninstallButton = new QPushButton("卸载");
    uninstallButton->setFixedSize(120, 35);
    connect(uninstallButton, &QPushButton::clicked, this, &MainWindow::onUninstallClicked);

    buttonLayout->addWidget(installButton);
    buttonLayout->addWidget(uninstallButton);
    layout->addLayout(buttonLayout);

    // 空白填充
    layout->addStretch(1);

    mainTabWidget->addTab(installWidget, "安装/卸载");
}

void MainWindow::createSettingsTab() {
    QWidget *settingsWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(settingsWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    // 透明度设置
    QWidget *transparencyWidget = new QWidget();
    QVBoxLayout *transparencyLayout = new QVBoxLayout(transparencyWidget);
    transparencyLayout->setSpacing(10);

    QLabel *transparencyLabel = new QLabel("透明度设置:");
    transparencyLabel->setStyleSheet("font-weight: bold;");
    transparencyLayout->addWidget(transparencyLabel);

    QHBoxLayout *sliderLayout = new QHBoxLayout();
    transparencySlider = new QSlider(Qt::Horizontal);
    transparencySlider->setRange(0, 100);
    transparencySlider->setValue(50);
    QLabel *valueLabel = new QLabel("50%");
    connect(transparencySlider, &QSlider::valueChanged, [valueLabel](int value) {
        valueLabel->setText(QString("%1%").arg(value));
    });

    sliderLayout->addWidget(transparencySlider);
    sliderLayout->addWidget(valueLabel);
    transparencyLayout->addLayout(sliderLayout);

    layout->addWidget(transparencyWidget);

    // 主题选择
    QWidget *themeWidget = new QWidget();
    QVBoxLayout *themeLayout = new QVBoxLayout(themeWidget);
    themeLayout->setSpacing(10);

    QLabel *themeLabel = new QLabel("主题选择:");
    themeLabel->setStyleSheet("font-weight: bold;");
    themeLayout->addWidget(themeLabel);

    themeComboBox = new QComboBox();
    themeComboBox->addItem("默认主题");
    themeComboBox->addItem("深色主题");
    themeComboBox->addItem("浅色主题");
    themeLayout->addWidget(themeComboBox);

    layout->addWidget(themeWidget);

    // 空白填充
    layout->addStretch(1);

    // 应用按钮
    applyButton = new QPushButton("应用设置");
    applyButton->setFixedSize(120, 35);
    connect(applyButton, &QPushButton::clicked, this, &MainWindow::onApplySettingsClicked);
    layout->addWidget(applyButton, 0, Qt::AlignCenter);

    // 更新检查按钮
    QPushButton *checkUpdateButton = new QPushButton("检查更新");
    checkUpdateButton->setFixedSize(120, 35);
    connect(checkUpdateButton, &QPushButton::clicked, this, &MainWindow::checkUpdateClicked);
    layout->addWidget(checkUpdateButton, 0, Qt::AlignCenter);

    // 更新进度条
    updateProgressBar = new QProgressBar();
    updateProgressBar->setVisible(false);
    layout->addWidget(updateProgressBar);
}

void MainWindow::createAboutTab() {
    QWidget *aboutWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(aboutWidget);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);
    layout->setAlignment(Qt::AlignCenter);

    // 应用图标
    QLabel *iconLabel = new QLabel();
    iconLabel->setPixmap(QPixmap(32, 32)); // 实际应用中应替换为真实图标
    iconLabel->setFixedSize(64, 64);
    iconLabel->setStyleSheet("background-color: #007acc; border-radius: 8px;");
    layout->addWidget(iconLabel, 0, Qt::AlignCenter);

    // 应用名称和版本
    QLabel *nameLabel = new QLabel("TranslucentSM");
    nameLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    layout->addWidget(nameLabel, 0, Qt::AlignCenter);

    QLabel *versionLabel = new QLabel("版本 1.0.0");
    versionLabel->setStyleSheet("color: #b0b0b0;");
    layout->addWidget(versionLabel, 0, Qt::AlignCenter);

    // 空白分隔
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #555555;");
    layout->addWidget(line);

    // 作者信息
    QLabel *authorLabel = new QLabel("作者: Yzy15");
    layout->addWidget(authorLabel, 0, Qt::AlignCenter);

    QLabel *githubLabel = new QLabel(
        "<a href='https://github.com/mc-yzy15/TranslucentSM-Legacy-Reborn'>GitHub 仓库</a>");
    githubLabel->setOpenExternalLinks(true);
    layout->addWidget(githubLabel, 0, Qt::AlignCenter);

    // 版权信息
    QLabel *copyrightLabel = new QLabel("© 2023 All Rights Reserved");
    copyrightLabel->setStyleSheet("color: #b0b0b0; font-size: 10px;");
    layout->addWidget(copyrightLabel, 0, Qt::AlignCenter);

    mainTabWidget->addTab(aboutWidget, "关于");
}

void MainWindow::onInstallClicked() {
    QString installPath = getInstallPath();
    QDir installDir(installPath);

    if (!installDir.exists()) {
        if (!installDir.mkpath(installPath)) {
            QMessageBox::critical(this, "错误", "无法创建安装目录！");
            return;
        }
    }

    statusLabel->setText("正在安装...");
    progressBar->setVisible(true);
    progressBar->setValue(0);
    installButton->setEnabled(false);
    uninstallButton->setEnabled(false);

    // 直接调用安装函数
    int result = installTranslucentSM(installPath);

    if (result == 0) {
        progressBar->setValue(100);
        statusLabel->setText("安装成功");
        QMessageBox::information(this, "成功", "安装已成功完成！");
        updateUIState();
    } else {
        statusLabel->setText("安装失败");
        QMessageBox::critical(this, "错误", "安装失败，请检查日志获取详细信息。");
    }

    progressBar->setVisible(false);
    installButton->setEnabled(true);
    uninstallButton->setEnabled(true);
}

void MainWindow::onUninstallClicked() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认", "确定要卸载 TranslucentSM 吗？",
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    statusLabel->setText("正在卸载...");
    progressBar->setVisible(true);
    progressBar->setValue(0);
    installButton->setEnabled(false);
    uninstallButton->setEnabled(false);

    // 直接调用卸载函数
    int result = uninstallTranslucentSM();

    if (result == 0) {
        progressBar->setValue(100);
        statusLabel->setText("卸载成功");
        QMessageBox::information(this, "成功", "卸载已成功完成！");
        updateUIState();
    } else {
        statusLabel->setText("卸载失败");
        QMessageBox::critical(this, "错误", "卸载失败，请检查日志获取详细信息。");
    }

    progressBar->setVisible(false);
    installButton->setEnabled(true);
    uninstallButton->setEnabled(true);
}

void MainWindow::onApplySettingsClicked() {
    // 保存设置
    QSettings settings("TranslucentSM", "ConfigTool");
    int transparencyValue = transparencySlider->value();
    settings.setValue("transparency", transparencyValue);
    settings.setValue("theme", themeComboBox->currentIndex());

    // 应用透明度设置到开始菜单进程
    TranslucentSM translucentSM;
    translucentSM.applyTransparencyToProcess("StartMenuExperienceHost.exe", transparencyValue);

    statusLabel->setText("设置已应用");
    QMessageBox::information(this, "成功", "透明度设置已应用！");
}

void MainWindow::onBrowseClicked() {
    QString path = QFileDialog::getExistingDirectory(this, "选择安装目录",
                                                     installPathEdit->text());
    if (!path.isEmpty()) {
        installPathEdit->setText(QDir::toNativeSeparators(path));
    }
}

void MainWindow::updateProgress(int value) {
    progressBar->setValue(value);
    statusLabel->setText(QString("正在处理... %1%").arg(value));
}

void MainWindow::onInstallationFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        statusLabel->setText("操作完成");
        QMessageBox::information(this, "成功", "操作已成功完成！");
        updateUIState();
    } else {
        statusLabel->setText("操作失败");
        QMessageBox::critical(this, "错误", "操作失败，请检查日志获取详细信息。");
    }
    progressBar->setVisible(false);
    installButton->setEnabled(true);
    uninstallButton->setEnabled(true);
}

bool MainWindow::checkInstallationStatus() {
    // 在实际应用中，这里应该检查软件是否已安装
    // 这里简单返回false模拟未安装状态
    return false;
}

void MainWindow::updateUIState() {
    bool isInstalled = checkInstallationStatus();
    installButton->setEnabled(!isInstalled);
    uninstallButton->setEnabled(isInstalled);
    applyButton->setEnabled(isInstalled);

    if (isInstalled) {
        statusLabel->setText("已安装：TranslucentSM");
    } else {
        statusLabel->setText("未安装");
    }
}

QString MainWindow::getInstallPath() {
    return installPathEdit->text();
}

private:
    Ui::MainWindow *ui;
    QProcess *installProcess;
    QProcess *uninstallProcess;
    QNetworkAccessManager *networkManager;
    QProgressBar *updateProgressBar;
    QString currentVersion = "1.0.0";
}

void MainWindow::checkUpdateClicked() {
    statusLabel->setText("正在检查更新...");
    
    if (!networkManager) {
        networkManager = new QNetworkAccessManager(this);
    }
    
    QUrl url("https://api.github.com/repos/mc-yzy15/TranslucentSM-Legacy-Reborn/releases");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "TranslucentSM");
    
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onUpdateCheckFinished);
    networkManager->get(request);
}

void MainWindow::onUpdateCheckFinished(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        statusLabel->setText("检查更新失败: " + reply->errorString());
        reply->deleteLater();
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray releases = doc.array();
    
    if (!releases.isEmpty()) {
        QJsonObject latestRelease = releases.first().toObject();
        QString latestVersion = latestRelease["tag_name"].toString().remove("v");
        QString downloadUrl = latestRelease["assets"].toArray().first().toObject()["browser_download_url"].toString();
        
        // 版本比较
        if (versionCompare(latestVersion, currentVersion) > 0) {
            QMessageBox::StandardButton replyBtn;
            replyBtn = QMessageBox::question(this, "发现更新", 
                                         QString("有新版本可用: %1\n是否下载并安装?").arg(latestVersion),
                                         QMessageBox::Yes|QMessageBox::No);
            
            if (replyBtn == QMessageBox::Yes) {
                statusLabel->setText("正在下载更新...");
                updateProgressBar->setVisible(true);
                updateProgressBar->setValue(0);
                
                connect(networkManager, &QNetworkAccessManager::downloadProgress, 
                        this, &MainWindow::onDownloadProgress);
                connect(networkManager, &QNetworkAccessManager::finished, 
                        this, &MainWindow::onDownloadFinished);
                
                QNetworkRequest downloadRequest(QUrl(downloadUrl));
                networkManager->get(downloadRequest);
            }
        } else {
            statusLabel->setText("当前已是最新版本");
        }
    } else {
        statusLabel->setText("未找到发布版本");
    }
    
    reply->deleteLater();
}

void MainWindow::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal > 0) {
        int percent = (bytesReceived * 100) / bytesTotal;
        updateProgressBar->setValue(percent);
        statusLabel->setText(QString("正在下载更新: %1%").arg(percent));
    }
}

void MainWindow::onDownloadFinished(QNetworkReply *reply) {
    updateProgressBar->setVisible(false);
    
    if (reply->error() == QNetworkReply::NoError) {
        QString tempPath = QDir::tempPath() + "/TranslucentSM_Update.zip";
        QFile file(tempPath);
        
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            
            // 启动安装程序
            QProcess::startDetached("explorer.exe", QStringList() << tempPath);
            qApp->quit();
        } else {
            statusLabel->setText("无法保存更新文件");
        }
    } else {
        statusLabel->setText("更新下载失败: " + reply->errorString());
    }
    
    reply->deleteLater();
}

int MainWindow::versionCompare(const QString &version1, const QString &version2) {
    QStringList v1 = version1.split(".");
    QStringList v2 = version2.split(".");
    
    int maxLen = qMax(v1.size(), v2.size());
    
    for (int i = 0; i < maxLen; ++i) {
        int num1 = i < v1.size() ? v1[i].toInt() : 0;
        int num2 = i < v2.size() ? v2[i].toInt() : 0;
        
        if (num1 > num2) return 1;
        if (num1 < num2) return -1;
    }
    
    return 0;
}
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QProcess>
#include <QLineEdit>
#include <QNetworkReply>
#include <QMouseEvent>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QTabWidget>
#include <QNetworkAccessManager>
#include <QFrame>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QStyleFactory>

// 自定义标题栏部件
class TitleBar : public QWidget {
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);
    void retranslateUi();

signals:
    void minimizeWindow();
    void closeWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QLabel *titleLabel;
    QPushButton *minimizeBtn;
    QPushButton *closeBtn;
    QPoint m_startPos;
    bool m_moving;
};

// 主窗口类
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void applyLanguage(const QString &languageCode);

signals:
    void languageChangeRequested(const QString &languageCode);

private:
    void updateUIState();
    void setupUI();
    void retranslateUi();
    void applyModernStyle();
    void createInstallationTab();
    bool checkInstallationStatus();
    void createSettingsTab();
    void createAboutTab();
    void setupLanguageSelector();
    void loadPersistedSettings();
    QString detectDefaultLanguage() const;
    QString normalizedLanguageCode(const QString &languageCode) const;

    QProgressBar* updateProgressBar;
    QProcess* installProcess;
    QNetworkAccessManager* networkManager;
    QString currentVersion;
    QString currentLanguageCode;
    QString selectedUpdateAssetName;

private slots:
    void onInstallClicked();
    void onUninstallClicked();
    void onApplySettingsClicked();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onUpdateCheckFinished(QNetworkReply *reply);
    void checkUpdateClicked();
    void onLanguageChanged(int index);

private:
    int versionCompare(const QString &version1, const QString &version2);
    void onBrowseClicked();
    void updateProgress(int value);
    void onInstallationFinished(int exitCode, QProcess::ExitStatus exitStatus);

    QString getInstallPath();
    // UI组件
    QWidget *mainCentralWidget;
    TitleBar *titleBar;
    QTabWidget *mainTabWidget;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QPushButton *installButton;
    QPushButton *uninstallButton;
    QPushButton *applyButton;
    QPushButton *browseButton;
    QPushButton *checkUpdateButton;
    QLineEdit *installPathEdit;
    QSlider *transparencySlider;
    QLabel *transparencyValueLabel;
    QComboBox *themeComboBox;
    QComboBox *languageComboBox;

    QWidget *installTabWidget;
    QWidget *settingsTabWidget;
    QWidget *aboutTabWidget;

    QLabel *pathLabel;
    QLabel *infoLabel;
    QLabel *transparencyLabel;
    QLabel *themeLabel;
    QLabel *languageLabel;
    QLabel *aboutNameLabel;
    QLabel *aboutVersionLabel;
    QLabel *aboutAuthorLabel;
    QLabel *aboutGithubLabel;
    QLabel *aboutCopyrightLabel;

};

#endif // MAINWINDOW_H

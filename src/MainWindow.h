#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include <QLineEdit>
#include <QNetworkReply>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QProgressBar>
#include <QFrame>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <QStyleFactory>

// 自定义标题栏部件
class TitleBar : public QWidget {
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);

signals:
    void minimizeWindow();
    void closeWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPoint m_startPos;
    bool m_moving;
};

// 主窗口类
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void updateUIState();
    bool checkInstallationStatus();
    void createSettingsTab();
    void createAboutTab();
    QProgressBar* updateProgressBar;
    QNetworkAccessManager* networkManager;
    QString currentVersion;
    QString getInstallPath();
private slots:
    void onInstallClicked();
    void onUninstallClicked();
    void onApplySettingsClicked();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onUpdateCheckFinished(QNetworkReply* reply);
    void checkUpdateClicked();
private:
    int versionCompare(const QString& version1, const QString& version2);
    void onBrowseClicked();
    void updateProgress(int value);
    void onInstallationFinished(int exitCode, QProcess::ExitStatus exitStatus);
};



private:
    void setupUI();
    void applyModernStyle();
    void createInstallationTab();
    void createSettingsTab();
    void createAboutTab();
    bool checkInstallationStatus();
    void updateUIState();
    QString getInstallPath();

    // UI组件
    QWidget *centralWidget;
    QTabWidget *mainTabWidget;
    QProgressBar *progressBar;
    QLabel *statusLabel;
    QPushButton *installButton;
    QPushButton *uninstallButton;
    QPushButton *applyButton;
    QLineEdit *installPathEdit;
    QSlider *transparencySlider;
    QComboBox *themeComboBox;
    QProcess *installProcess;
};

#endif // MAINWINDOW_H
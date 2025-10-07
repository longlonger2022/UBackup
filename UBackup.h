#pragma once

#include <QtWidgets/QWidget>
#include "ui_UBackup.h"
#include <QGroupBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QStorageInfo>
#include <QSystemTrayIcon>
#include <QEvent>
#include <QCloseEvent>
#include <QThread>
#include <QCheckBox>

class UBackup : public QWidget
{
    Q_OBJECT

public:
    UBackup(QWidget *parent = nullptr);
    ~UBackup();

private:
    Ui::UBackupClass ui;

    void BackupPathIsTrueFlagSet();
    void BackupPathIsTrueLabelSet();

    void readConfig();
    void writeConfig();

    void writeLog(QString logType, QString logText);

    // 检测U盘插入
    void checkUsb();

    // 备份函数
    void backup(QString fromPath, QString toPath);


    bool BackupPathIsTrueFlag;
    bool isBackuping;
    bool isShowMessageWhenBackupStart;
    bool isShowMessageWhenBackupEnd;
    bool isShowMessageWhenBackupError;

    QString BackupPath;
    
    QStorageInfo UDisk;

    QList<QStorageInfo> OldDiskList;
    QList<QStorageInfo> DiskList;

    QRect desktop;


    QFont font;

    // Widgets
    QWidget* AboutWidget;
    QWidget* HelpWidget;
    QWidget* SponsorWidget;
    QWidget* SettingsWidget;

    // TrayIcon
    QSystemTrayIcon* TrayIcon;

    // GroupBoxes
    QGroupBox* BackupPathGb;
    QGroupBox* StatusGb;
    QGroupBox* AboutGb;

    QGroupBox* SettingsBackupGb;

    // PushButtons
    QPushButton* ChoosePathBtn;
    QPushButton* AboutBtn;
    QPushButton* HelpBtn;
    QPushButton* SponsorBtn;
    QPushButton* SettingsBtn;
    QPushButton* ImidiateBackupBtn;


    // CheckBoxes
    QCheckBox* SettingsShowMessageWhenBackupStartCkbx;
    QCheckBox* SettingsShowMessageWhenBackupEndCkbx;
    QCheckBox* SettingsShowMessageWhenBackupErrorCkbx;

    // Labels
    QLabel* AboutIconLabel;
    QLabel* SponsorWidgetWeChatPayLabel;
    QLabel* BackupPathIsTrueLabel;
    QLabel* BackupStatusLabel;
    QLabel* AboutLabel;
    QLabel* SponsorLabel;

    // LineEdits
    QLineEdit* BackupPathLedt;

    // ProgressBars
    QProgressBar* BackupPgb;

    // TextEdits
    QTextEdit* AboutTedt;
    QTextEdit* HelpTedt;

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
};

#define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
#include "UBackup.h"
#include <QDesktopWidget>
#include <QIcon>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QTextStream>
#include <QStorageInfo>
#include <QDirIterator>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QThread>



UBackup::UBackup(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    //设置窗口居中
    desktop = QApplication::desktop()->screenGeometry();

    this->setWindowTitle(QString::fromUtf8("U备份"));
    this->resize(desktop.width() / 2, desktop.height() / 2);
    this->setGeometry(desktop.width() / 2 - this->width() / 2, desktop.height() / 2 - this->height() / 2, this->width(), this->height());
    this->setWindowIcon(QIcon("img/UBackup_icon.512px.png"));
    this->setStyleSheet("QWidget{font-family: DIN1451, Segoe UI, Source Han Sans, Microsoft YaHei UI;}");
    this->setMinimumSize(300, 200);





    BackupPathIsTrueFlag = false;
    BackupPath = "";

    isBackuping = false;

    isShowMessageWhenBackupStart = true;
    isShowMessageWhenBackupEnd = true;
    isShowMessageWhenBackupError = true;

    DiskList = QStorageInfo::mountedVolumes();
    OldDiskList = DiskList;


    QString SomethingInteresting = "Fuck QThread!";


    // Widgets
    AboutWidget = new QWidget();
    AboutWidget->setGeometry(desktop.width() / 4, desktop.height() / 4, desktop.width() / 2, desktop.height() / 2);
    AboutWidget->setWindowTitle("关于 - U备份");
    AboutWidget->setWindowIcon(QIcon("img/UBackup_icon.512px.png"));
    AboutWidget->setStyleSheet("QWidget{font-family: DIN1451, Segoe UI, Source Han Sans, Microsoft YaHei UI;}");
    AboutWidget->setMinimumSize(this->width() / 2, this->height() / 2);
    AboutWidget->installEventFilter(this);

    HelpWidget = new QWidget();
    HelpWidget->setGeometry(desktop.width() / 2 - this->width() / 4, desktop.height() / 2 - this->height() / 4, this->width() / 2, this->height() /2);
    HelpWidget->setWindowTitle("帮助 - U备份");
    HelpWidget->setWindowIcon(QIcon("img/UBackup_icon.512px.png"));
    HelpWidget->setStyleSheet("QWidget{font-family: DIN1451, Segoe UI, Source Han Sans, Microsoft YaHei UI;}");
    HelpWidget->installEventFilter(this);

    SponsorWidget = new QWidget();
    SponsorWidget->setGeometry(desktop.width() / 4, desktop.height() / 4, desktop.width() / 2, desktop.height() / 2);
    SponsorWidget->setWindowTitle("赞助 - U备份");
    SponsorWidget->setWindowIcon(QIcon("img/UBackup_icon.512px.png"));
    SponsorWidget->setStyleSheet("QWidget{font-family: DIN1451, Segoe UI, Source Han Sans, Microsoft YaHei UI;}");
    SponsorWidget->installEventFilter(this);

    SettingsWidget = new QWidget();
    SettingsWidget->setGeometry(desktop.width() * 3 / 8, desktop.height() / 4, desktop.width() / 4, desktop.height() / 2);
    SettingsWidget->setWindowTitle("设置 - U备份");
    SettingsWidget->setWindowIcon(QIcon("img/UBackup_icon.512px.png"));
    SettingsWidget->setStyleSheet("QWidget{font-family: DIN1451, Segoe UI, Source Han Sans, Microsoft YaHei UI;}");
    SettingsWidget->installEventFilter(this);




    // TrayIcon
    TrayIcon = new QSystemTrayIcon(this);
    TrayIcon->setIcon(QIcon("img/UBackup_icon.512px.png"));
    TrayIcon->setToolTip("U备份");
    // 当单击托盘图标时，显示主窗口
    connect(TrayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            this->showNormal();
        }
        });
    // 创建托盘菜单
    QMenu* trayIconMenu = new QMenu(this);
    QAction* titleAction = new QAction("U备份", this);
    titleAction->setIcon(QIcon("img/UBackup_icon.512px.png"));
    QAction* showAction = new QAction("显示主窗口", this);
    QAction* aboutAction = new QAction("关于", this);
    QAction* helpAction = new QAction("帮助", this);
    QAction* sponsorAction = new QAction("赞助", this);
    QAction* settingsAction = new QAction("设置", this);
    QAction* exitAction = new QAction("退出", this);
    connect(showAction, &QAction::triggered, this, &QWidget::showNormal);
    connect(aboutAction, &QAction::triggered, this, [=]() {
        AboutWidget->show();
        });
    connect(helpAction, &QAction::triggered, this, [=]() {
        HelpWidget->show();
        });
    connect(sponsorAction, &QAction::triggered, this, [=]() {
        SponsorWidget->show();
        });
    connect(settingsAction, &QAction::triggered, this, [=]() {
        SettingsWidget->show();
        });
    connect(exitAction, &QAction::triggered, this, [&] {
        TrayIcon->showMessage("U备份", "程序已退出", QSystemTrayIcon::Information);
        qApp->quit();
        });
    trayIconMenu->addAction(titleAction);
    trayIconMenu->addAction(showAction);
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addAction(helpAction);
    trayIconMenu->addAction(sponsorAction);
    trayIconMenu->addAction(settingsAction);
    trayIconMenu->addAction(exitAction);
    TrayIcon->setContextMenu(trayIconMenu);
    TrayIcon->show();





    // GroupBoxes
    BackupPathGb = new QGroupBox("备份路径", this);
    BackupPathGb->setGeometry(desktop.width() * 0.05, 0, this->width() - desktop.width() * 0.05, this->height() / 2 - 10);
    BackupPathGb->show();

    StatusGb = new QGroupBox("状态", this);
    StatusGb->setGeometry(desktop.width() * 0.05, this->height() / 2 + 10, this->width() - desktop.width() * 0.05, this->height() / 2);
    StatusGb->show();

    AboutGb = new QGroupBox("关于", AboutWidget);
    AboutGb->setGeometry(AboutWidget->width() * 0.5, AboutWidget->height() * 0.05, AboutWidget->width() * 0.45, AboutWidget->height() * 0.4);
    AboutGb->show();

    SettingsBackupGb = new QGroupBox("备份设置", SettingsWidget);
    SettingsBackupGb->setGeometry(SettingsWidget->width() * 0.05, SettingsWidget->height() * 0.05, SettingsWidget->width() * 0.9, SettingsWidget->height() * 0.9);
    SettingsBackupGb->show();

    // PushButtons
    AboutBtn = new QPushButton("关于", this);
    AboutBtn->setGeometry(0, 0, desktop.width() * 0.05, this->height() / 4);
    font.setPixelSize(AboutBtn->width() * 0.25);
    AboutBtn->setFont(font);
    AboutBtn->show();

    HelpBtn = new QPushButton("帮助", this);
    HelpBtn->setGeometry(0, this->height() / 4, desktop.width() * 0.05, this->height() / 4);
    HelpBtn->setFont(font);
    HelpBtn->show();

    SponsorBtn = new QPushButton("赞助", this);
    SponsorBtn->setGeometry(0, this->height() * 2 / 4, desktop.width() * 0.05, this->height() / 4);
    SponsorBtn->setFont(font);
    SponsorBtn->show();

    SettingsBtn = new QPushButton("设置", this);
    SettingsBtn->setGeometry(0, this->height() * 3 / 4, desktop.width() * 0.05, this->height() / 4);
    SettingsBtn->setFont(font);
    SettingsBtn->show();

    ChoosePathBtn = new QPushButton("选择路径", BackupPathGb);
    ChoosePathBtn->setGeometry(BackupPathGb->width() * 0.025, desktop.height() * 0.03 + 40, BackupPathGb->width() * 0.95, desktop.height() * 0.03);
    font.setPixelSize(ChoosePathBtn->height() * 0.5);
    ChoosePathBtn->setFont(font);
    ChoosePathBtn->show();

    ImidiateBackupBtn = new QPushButton("立即选择路径进行备份", StatusGb);
    ImidiateBackupBtn->setGeometry(StatusGb->width() * 0.025, desktop.height() * 0.06 + 50, StatusGb->width() * 0.95, desktop.height() * 0.03);
    ImidiateBackupBtn->setFont(font);
    ImidiateBackupBtn->show();


    // CheckBoxes
    SettingsShowMessageWhenBackupStartCkbx = new QCheckBox("备份开始时提示", SettingsBackupGb);
    SettingsShowMessageWhenBackupStartCkbx->setGeometry(SettingsBackupGb->width() * 0.05, SettingsBackupGb->height() * 0.05, SettingsBackupGb->width() * 0.9, desktop.height() * 0.03);
    SettingsShowMessageWhenBackupStartCkbx->setFont(font);
    SettingsShowMessageWhenBackupStartCkbx->setChecked(isShowMessageWhenBackupStart);
    SettingsShowMessageWhenBackupStartCkbx->show();
    SettingsShowMessageWhenBackupEndCkbx = new QCheckBox("备份结束时提示", SettingsBackupGb);
    SettingsShowMessageWhenBackupEndCkbx->setGeometry(SettingsBackupGb->width() * 0.05, SettingsBackupGb->height() * 0.1, SettingsBackupGb->width() * 0.9, desktop.height() * 0.03);
    SettingsShowMessageWhenBackupEndCkbx->setFont(font);
    SettingsShowMessageWhenBackupEndCkbx->setChecked(isShowMessageWhenBackupEnd);
    SettingsShowMessageWhenBackupEndCkbx->show();
    SettingsShowMessageWhenBackupErrorCkbx = new QCheckBox("备份出错时提示", SettingsBackupGb);
    SettingsShowMessageWhenBackupErrorCkbx->setGeometry(SettingsBackupGb->width() * 0.05, SettingsBackupGb->height() * 0.15, SettingsBackupGb->width() * 0.9, desktop.height() * 0.03);
    SettingsShowMessageWhenBackupErrorCkbx->setFont(font);
    SettingsShowMessageWhenBackupErrorCkbx->setChecked(isShowMessageWhenBackupError);
    SettingsShowMessageWhenBackupErrorCkbx->show();


    // Labels
    AboutIconLabel = new QLabel(AboutWidget);
    AboutIconLabel->setGeometry((AboutWidget->width() * 0.9 - AboutWidget->height() * 0.4) / 4, AboutWidget->height() * 0.05, AboutWidget->height() * 0.4, AboutWidget->height() * 0.4);
    AboutIconLabel->setPixmap(QPixmap("img/UBackup_icon.512px.png"));
    AboutIconLabel->setScaledContents(true);
    AboutIconLabel->show();
    SponsorWidgetWeChatPayLabel = new QLabel(SponsorWidget);
    SponsorWidgetWeChatPayLabel->setGeometry(SponsorWidget->width() - SponsorWidget->height() * 1242 / 1692, 0, SponsorWidget->height() * 1242 / 1692, SponsorWidget->height());
    SponsorWidgetWeChatPayLabel->setPixmap(QPixmap("img/QRCode/WeChatPay.png"));
    SponsorWidgetWeChatPayLabel->setScaledContents(true);
    SponsorWidgetWeChatPayLabel->show();
    BackupPathIsTrueLabel = new QLabel("路径情况", BackupPathGb);
    BackupPathIsTrueLabel->setGeometry(BackupPathGb->width() * 0.025, desktop.height() * 0.03 + 80, BackupPathGb->width() * 0.95, desktop.height() * 0.03);
    BackupPathIsTrueLabel->setStyleSheet("QLabel{color: black;}");
    BackupPathIsTrueLabel->setFont(font);
    BackupPathIsTrueLabel->show();
    BackupStatusLabel = new QLabel("状态", StatusGb);
    BackupStatusLabel->setGeometry(StatusGb->width() * 0.025, desktop.height() * 0.03 + 40, StatusGb->width() * 0.95, desktop.height() * 0.03);
    BackupStatusLabel->setStyleSheet("QLabel{color: black;}");
    BackupStatusLabel->setFont(font);
    BackupStatusLabel->show();

    AboutLabel = new QLabel(AboutGb);
    AboutLabel->setText("产品名称：U备份 (UBackup)\n版本：1.1.0\nCopyright © 2025 龙ger_longer");
    AboutLabel->setGeometry(AboutGb->width() * 0.05, AboutGb->height() * 0.1, AboutGb->width() * 0.95, AboutGb->height() * 0.9);
    AboutLabel->setStyleSheet("QLabel{color: black;}");
    AboutLabel->setFont(font);
    AboutLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop); // 靠左向上对齐
    AboutLabel->setWordWrap(true); // 自动换行
    AboutLabel->show();
    SponsorLabel = new QLabel(SponsorWidget);
    SponsorLabel->setText("欢迎赞助！\n您的赞助是我们前进的动力！");
    SponsorLabel->setGeometry(SponsorWidget->width() * 0.05, SponsorWidget->height() * 0.05, SponsorWidget->width() - SponsorWidget->height() * 1242 / 1692, SponsorWidget->height() * 0.6);
    SponsorLabel->setStyleSheet("QLabel{color: black;}");
    SponsorLabel->setFont(font);
    SponsorLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop); // 靠左向上对齐
    SponsorLabel->setWordWrap(true); // 自动换行
    SponsorLabel->show();

    // LineEdits
    BackupPathLedt = new QLineEdit(BackupPathGb);
    BackupPathLedt->setGeometry(BackupPathGb->width() * 0.025, 30, BackupPathGb->width() * 0.95, desktop.height() * 0.03);
    BackupPathLedt->setPlaceholderText("请输入备份路径");
    font.setPixelSize(BackupPathLedt->height() * 0.5);
    BackupPathLedt->setFont(font);
    BackupPathLedt->show();


    // TextEdits
    AboutTedt = new QTextEdit(AboutWidget);
    AboutTedt->setGeometry(AboutWidget->width() * 0.05, AboutWidget->height() * 0.5, AboutWidget->width() * 0.9, AboutWidget->height() * 0.45);
    AboutTedt->setText("本软件遵循GPLv3开源协议。\n本软件本身完全免费，赞助属于自愿，转载本软件需标明出处，严禁倒卖。\n介绍：这是一个实用的U盘备份工具，可以将您的U盘数据备份到本地，时刻保障您的数据安全。");
    AboutTedt->setReadOnly(true);
    font.setPixelSize(desktop.height() * 0.015);
    AboutTedt->setFont(font);
    AboutTedt->show();
    HelpTedt = new QTextEdit(HelpWidget);
    HelpTedt->setGeometry(0, 0, HelpWidget->width(), HelpWidget->height());
    HelpTedt->setText("使用方法：\n1. 点击“选择路径”按钮，选择一个本地路径作为备份路径。\n2. 插入U盘，将自动开始备份。\n3. 备份完成后，您可以在备份路径中找到您备份的文件。\n\n注意事项：\n1. 请确保您的U盘是正常的。\n2. 请确保您有权限读取U盘并有权限写入备份路径。\n3. 请确保您的备份路径所在的磁盘拥有足够的空间用于备份。\n4. 备份过程中请不要拔出您的U盘。\n\n如果觉得本软件做得不错，欢迎赞助！(纯自愿)");
    HelpTedt->setReadOnly(true);
    HelpTedt->setFont(font);
    HelpTedt->show();



    // ProgressBars
    BackupPgb = new QProgressBar(StatusGb);
    BackupPgb->setGeometry(StatusGb->width() * 0.025, desktop.height() * 0.03, StatusGb->width() * 0.95, desktop.height() * 0.03);
    BackupPgb->setRange(0, 100);
    BackupPgb->setValue(0);
    BackupPgb->setTextVisible(false);
    BackupPgb->show();




    readConfig();




    // connects
    // 每隔3秒更新
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [&] {
        checkUsb();
        });
    timer->start(3000);
    connect(AboutBtn, &QPushButton::clicked, [&]{ AboutWidget->show(); });
    connect(HelpBtn, &QPushButton::clicked, [&]{ HelpWidget->show(); });
    connect(SponsorBtn, &QPushButton::clicked, [&]{ SponsorWidget->show(); });
    connect(SettingsBtn, &QPushButton::clicked, [&]{ SettingsWidget->show(); });
    connect(BackupPathLedt, &QLineEdit::textChanged, this, [&] {
        BackupPath = BackupPathLedt->text();
        writeLog("INFO", "修改了备份路径为：" + BackupPath);
        BackupPathIsTrueFlagSet();
        writeConfig();
        });
    connect(ChoosePathBtn, &QPushButton::clicked, [&] {
        BackupPathLedt->setText(QFileDialog::getExistingDirectory(this, "选择备份路径", BackupPath, QFileDialog::ShowDirsOnly));
        });
    connect(ImidiateBackupBtn, &QPushButton::clicked, [&] {
        QString path = QFileDialog::getExistingDirectory(this, "选择U盘路径", "/", QFileDialog::ShowDirsOnly);
        if (path.isEmpty()) {
            QMessageBox::critical(this, "提示", "未选择U盘路径");
        } else {
            UDisk = QStorageInfo(path);
            backup(UDisk.rootPath(), BackupPath);
        }
        });
    connect(SettingsShowMessageWhenBackupStartCkbx, &QCheckBox::stateChanged, this, [&] {
        isShowMessageWhenBackupStart = SettingsShowMessageWhenBackupStartCkbx->isChecked();
        writeLog("INFO", "修改了[设置备份开始时提示]为：" + QString::number(isShowMessageWhenBackupStart));
        writeConfig();
        });
    connect(SettingsShowMessageWhenBackupEndCkbx, &QCheckBox::stateChanged, this, [&] {
        isShowMessageWhenBackupEnd = SettingsShowMessageWhenBackupEndCkbx->isChecked();
        writeLog("INFO", "修改了[设置备份结束时提示]为：" + QString::number(isShowMessageWhenBackupEnd));
        writeConfig();
        });
    connect(SettingsShowMessageWhenBackupErrorCkbx, &QCheckBox::stateChanged, this, [&] {
        isShowMessageWhenBackupError = SettingsShowMessageWhenBackupErrorCkbx->isChecked();
        writeLog("INFO", "修改了[设置备份错误时提示]为：" + QString::number(isShowMessageWhenBackupError));
        writeConfig();
        });
}

UBackup::~UBackup()
{}


// 判断路径是否正常标志函数
void UBackup::BackupPathIsTrueFlagSet() {
    if(!BackupPathLedt->text().isEmpty()) {
        if(QDir(BackupPathLedt->text()).exists()) {
            BackupPathIsTrueFlag = true;
        } else {
            BackupPathIsTrueFlag = false;
        }
    } else {
        BackupPathIsTrueFlag = false;
    }
    BackupPathIsTrueLabelSet();
}

// 设置路径Label文本函数
void UBackup::BackupPathIsTrueLabelSet() {
    if (BackupPathIsTrueFlag) {
        BackupPathIsTrueLabel->setText("✓ 路径正常");
        BackupPathIsTrueLabel->setStyleSheet("QLabel{color: green;}");
        BackupStatusLabel->setText("准备就绪");
        BackupStatusLabel->setStyleSheet("QLabel{color: green;}");
        writeLog("INFO", "检测路径，路径正常");
    }
    else {
        if(BackupPath.isEmpty()) {
            BackupPathIsTrueLabel->setText("× 路径不能为空");
            BackupStatusLabel->setText("未准备就绪，请输入路径");
            writeLog("WARNING", "检测路径，路径为空");
        } 
        else if (!QDir(BackupPath).exists()) {
            BackupPathIsTrueLabel->setText("× 路径不存在");
            BackupStatusLabel->setText("未准备就绪，路径不存在");
            writeLog("WARNING", "检测路径，路径不存在");
        }
        else {
            BackupPathIsTrueLabel->setText("× 路径异常");
            BackupStatusLabel->setText("未准备就绪，路径异常");
            writeLog("WARNING", "检测路径，路径异常");
        }
        BackupPathIsTrueLabel->setStyleSheet("QLabel{color: red;}");
        BackupStatusLabel->setStyleSheet("QLabel{color: red;}");
    }
}




// 读取配置文件
void UBackup::readConfig() {
    if (QFile::exists("config.ini")) {
        QSettings settings("config.ini", QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        BackupPath = settings.value("BackupPath").toString();
        writeLog("INFO", "读取配置文件成功");
        BackupPathLedt->setText(BackupPath);
        BackupPathIsTrueFlagSet();
        isShowMessageWhenBackupStart = settings.value("isShowMessageWhenBackupStart").toBool();
        SettingsShowMessageWhenBackupStartCkbx->setChecked(isShowMessageWhenBackupStart);
        isShowMessageWhenBackupEnd = settings.value("isShowMessageWhenBackupEnd").toBool();
        SettingsShowMessageWhenBackupEndCkbx->setChecked(isShowMessageWhenBackupEnd);
        isShowMessageWhenBackupError = settings.value("isShowMessageWhenBackupError").toBool();
        SettingsShowMessageWhenBackupErrorCkbx->setChecked(isShowMessageWhenBackupError);
    } else {
        QMessageBox::critical(this, "提示", "未找到配置文件，将使用默认配置。");
        writeLog("WARNING", "未找到配置文件，使用默认配置");
    }

}
// 写入配置文件
void UBackup::writeConfig() {
    QSettings settings("config.ini", QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    settings.setValue("BackupPath", BackupPath);
    settings.setValue("isShowMessageWhenBackupStart", isShowMessageWhenBackupStart);
    settings.setValue("isShowMessageWhenBackupEnd", isShowMessageWhenBackupEnd);
    settings.setValue("isShowMessageWhenBackupError", isShowMessageWhenBackupError);
    settings.sync();
    writeLog("INFO", "写入配置文件成功");
}

// 写入日志文件
void UBackup::writeLog(QString logType, QString logText) {
    QFile file("UBackup.log");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << " - " << logType << " - " << logText << "\n";
        file.close();
    }
}

// 检测U盘插入
void UBackup::checkUsb() {
    DiskList = QStorageInfo::mountedVolumes();
    writeLog("INFO", "检测一次");
    if (DiskList != OldDiskList) {
        writeLog("INFO", "1. 检测到U盘变化");
        if (DiskList.size() > OldDiskList.size()) {
            writeLog("INFO", "2. 检测到U盘插入");
            for (int i = 0; i < DiskList.size(); i++) {
                if (!OldDiskList.contains(DiskList[i])) {
                    writeLog("INFO", "3. 处理插入的U盘");
                    UDisk = DiskList[i];
                    backup(UDisk.rootPath(), BackupPath);
                }
            }
        }
        OldDiskList = DiskList;
    }
}



// 备份函数
void UBackup::backup(QString fromPath, QString toPath) {
    toPath += "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh'h-'mm'min-'ss's'") + " " + UDisk.displayName() + "(" + UDisk.rootPath()[0] + ")/";
    BackupStatusLabel->setText(toPath);
    QDir().mkpath(toPath);
    if (BackupPathIsTrueFlag) {
        if (isShowMessageWhenBackupStart) TrayIcon->showMessage("UBackup", "开始备份", QSystemTrayIcon::Information);
        BackupStatusLabel->setText("正在备份...");
        BackupStatusLabel->setStyleSheet("QLabel{color: black;}");
        BackupPgb->setRange(0, 0);
        BackupPathGb->setDisabled(true);
        ImidiateBackupBtn->setDisabled(true);
        writeLog("INFO", "开始备份，将 " + fromPath + " 备份到 " + BackupPath);
        // 复制U盘文件到备份路径
        QDirIterator it(fromPath, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString filePath = it.next();
            QString destPath = toPath + filePath.mid(fromPath.length());
            if (!QFileInfo(filePath).isFile()) {
                QDir().mkpath(destPath);
            }
            else {
                QFile::copy(filePath, destPath);
            }
        }
        if (isShowMessageWhenBackupEnd) TrayIcon->showMessage("UBackup", "备份结束", QSystemTrayIcon::Information);
        BackupPgb->setRange(0, 1);
        BackupPgb->setValue(1);
        BackupStatusLabel->setText("备份完成");
        BackupStatusLabel->setStyleSheet("QLabel{color: green;}");
        BackupPathGb->setDisabled(false);
        ImidiateBackupBtn->setDisabled(false);
        // 等待三秒恢复平时状态
        QTimer::singleShot(3000, [&] {
            BackupPathIsTrueFlagSet();
            BackupPgb->setRange(0, 1);
            BackupPgb->setValue(0);
            });
        writeLog("INFO", "备份完成");
    }
    else {
        if (isShowMessageWhenBackupError) TrayIcon->showMessage("UBackup", "备份失败，路径不存在", QSystemTrayIcon::Critical);
        writeLog("ERROR", "备份路径异常，无法备份");
    }
}




void UBackup::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    // 备份路径分组框
    BackupPathGb->setGeometry(desktop.width() * 0.05, 0, this->width() - desktop.width() * 0.05, this->height() / 2 - 10);
    BackupPathLedt->setGeometry(BackupPathGb->width() * 0.025, 30, BackupPathGb->width() * 0.95, desktop.height() * 0.03);
    ChoosePathBtn->setGeometry(BackupPathGb->width() * 0.025, BackupPathLedt->y() + BackupPathLedt->height() + 10, BackupPathGb->width() * 0.95, desktop.height() * 0.03);
    BackupPathIsTrueLabel->setGeometry(BackupPathGb->width() * 0.025, ChoosePathBtn->y() + ChoosePathBtn->height() + 10, BackupPathGb->width() * 0.95, desktop.height() * 0.03);
    // 状态分组框
    StatusGb->setGeometry(desktop.width() * 0.05, this->height() / 2 + 10, this->width() - desktop.width() * 0.05, this->height() / 2);
    BackupPgb->setGeometry(StatusGb->width() * 0.025, 30, StatusGb->width() * 0.95, desktop.height() * 0.03);
    BackupStatusLabel->setGeometry(StatusGb->width() * 0.025, BackupPgb->y() + BackupPgb->height() + 10, StatusGb->width() * 0.95, desktop.height() * 0.03);
    ImidiateBackupBtn->setGeometry(StatusGb->width() * 0.025, BackupStatusLabel->y() + BackupStatusLabel->height() + 10, StatusGb->width() * 0.95, desktop.height() * 0.03);

    AboutBtn->setGeometry(0, 0, desktop.width() * 0.05, this->height() / 4);
    HelpBtn->setGeometry(0, this->height() / 4, desktop.width() * 0.05, this->height() / 4);
    SponsorBtn->setGeometry(0, this->height() * 2 / 4, desktop.width() * 0.05, this->height() / 4);
    SettingsBtn->setGeometry(0, this->height() * 3 / 4, desktop.width() * 0.05, this->height() / 4);
    this->setMinimumSize(desktop.width() * 0.2, desktop.height() * 0.15 + 190);
}

void UBackup::closeEvent(QCloseEvent* event) {
    QWidget::closeEvent(event);
    // 最小化到系统托盘
    if (event->spontaneous() && TrayIcon->isVisible()) {
        TrayIcon->showMessage("UBackup", "程序已最小化到系统托盘", QSystemTrayIcon::Information);
    }
    else {
        event->accept();
    }
}

bool UBackup::eventFilter(QObject* obj, QEvent* event) {
    if (obj == AboutWidget && event->type() == QEvent::Resize) {
        AboutWidget->setMinimumWidth(AboutWidget->height());
        AboutGb->setGeometry(AboutWidget->width() * 0.5, AboutWidget->height() * 0.05, AboutWidget->width() * 0.45, AboutWidget->height() * 0.4);
        AboutLabel->setGeometry(AboutGb->width() * 0.05, AboutGb->height() * 0.1, AboutGb->width() * 0.95, AboutGb->height() * 0.9);
        AboutIconLabel->setGeometry((AboutWidget->width() * 0.8 - AboutWidget->height() * 0.4) / 4, AboutWidget->height() * 0.05, AboutWidget->height() * 0.4, AboutWidget->height() * 0.4);
        AboutTedt->setGeometry(AboutWidget->width() * 0.05, AboutWidget->height() * 0.5, AboutWidget->width() * 0.9, AboutWidget->height() * 0.45);
    }
    if (obj == SponsorWidget && event->type() == QEvent::Resize) {
        SponsorWidget->setMinimumWidth(SponsorWidget->height() * 1242 / 1692 * 3 / 2);
        SponsorLabel->setGeometry(SponsorWidget->width() * 0.05, SponsorWidget->height() * 0.05, SponsorWidget->width() * 0.95 - SponsorWidget->height() * 1242 / 1692, SponsorWidget->height() * 0.95);
        SponsorWidgetWeChatPayLabel->setGeometry(SponsorWidget->width() - SponsorWidget->height() * 1242 / 1692, 0, SponsorWidget->height() * 1242 / 1692, SponsorWidget->height());
    }
    if (obj == HelpWidget && event->type() == QEvent::Resize) {
        HelpTedt->setGeometry(0, 0, HelpWidget->width(), HelpWidget->height());
    }
    if (obj == SettingsWidget && event->type() == QEvent::Resize) {
        SettingsBackupGb->setGeometry(SettingsWidget->width() * 0.05, SettingsWidget->height() * 0.05, SettingsWidget->width() * 0.9, SettingsWidget->height() * 0.9);
        SettingsShowMessageWhenBackupStartCkbx->setGeometry(SettingsBackupGb->width() * 0.05, desktop.height() * 0.03, SettingsBackupGb->width() * 0.9, desktop.height() * 0.03);
        SettingsShowMessageWhenBackupEndCkbx->setGeometry(SettingsBackupGb->width() * 0.05, SettingsShowMessageWhenBackupStartCkbx->y() + SettingsShowMessageWhenBackupStartCkbx->height() + 10, SettingsBackupGb->width() * 0.9, desktop.height() * 0.03);
        SettingsShowMessageWhenBackupErrorCkbx->setGeometry(SettingsBackupGb->width() * 0.05, SettingsShowMessageWhenBackupEndCkbx->y() + SettingsShowMessageWhenBackupEndCkbx->height() + 10, SettingsBackupGb->width() * 0.9, desktop.height() * 0.03);
        SettingsWidget->setMinimumSize(desktop.width() / 8, SettingsBackupGb->y() + SettingsShowMessageWhenBackupErrorCkbx->y() + SettingsShowMessageWhenBackupErrorCkbx->height() * 2);
    }


    return QWidget::eventFilter(obj, event);
}
#include "UBackup.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setOrganizationName("U备份");
    QApplication::setOrganizationDomain("https://github.com/longlonger2022/UBackup");
    QApplication::setApplicationName("U备份");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setWindowIcon(QIcon("img/UBackup_icon.512px.png"));
    // 关闭事件
    QApplication::setQuitOnLastWindowClosed(false);
    UBackup w;
    w.show();
    return a.exec();
}

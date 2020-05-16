#include "widget.h"
#include <QApplication>
#include <QFile>
#include <QIcon>

class CommonHelper
{
public:
    static void setStyle(const QString &style) {//打开样式表
        QFile qss(style);
        qss.open(QFile::ReadOnly);
        qApp->setStyleSheet(qss.readAll());
        qss.close();
    }
};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString strPath=QApplication::applicationDirPath();
    strPath +="/img/server.png";
    a.setWindowIcon(QIcon(strPath));
    Widget w;
    w.setWindowTitle("MiToo-S");
    w.show();
    // 加载QSS样式
    CommonHelper::setStyle("black_style.qss");
    return a.exec();
}

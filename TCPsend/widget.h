#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QFile>
#include <string>
#include <QBuffer>


namespace Ui {

class Widget;

}

class Widget : public QWidget

{

    Q_OBJECT

public:

    QFile *localFile_current;//指向当前的文件

    explicit Widget(QWidget *parent = 0);

    ~Widget();

private:

    Ui::Widget *ui;

    QTcpSocket *tcpClient;

    //QFile *localFile_current;//指向当前的文件

    QFile *localFile;//指向打开的文件

    QString fileName;  //文件名

    QByteArray outBlock;  //分次传

    qint64 loadSize;  //每次发送数据的大小

    qint64 byteToWrite;  //剩余数据大小

    qint64 totalSize;  //文件总大小

    int sendTimes;  //用来标记是否为第一次发送，第一次以后连接信号触发，后面的则手动调用

    bool isconnected = false;

private slots:

    void send();  //传送文件头信息
    void tcp_connected();
    void dis_connected();
    void goOnSend(qint64);  //传送文件内容

    QString getHostIpAddress();//获取本机IP地址
    QImage toGray(QImage image);//灰度转换函数
    QImage locked(QString lock_name);//加密函数

    void on_pushButton_clicked();//打开文件
    void on_pushButton_2_clicked();//发送文件
    void on_pushButton_7_clicked();//刷新IP地址
    void on_pushButton_8_clicked();//转灰度图像按钮
    void on_pushButton_9_clicked();//显示原图
    void on_pushButton_3_clicked();//加密按钮
    void on_pushButton_6_clicked();//解密按钮
};

#endif // WIDGET_H

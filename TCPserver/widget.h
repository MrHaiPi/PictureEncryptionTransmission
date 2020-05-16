#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QFile>
#include <QString>
#include <QDir>

namespace Ui {
class Widget;
}
class Widget : public QWidget

{

    Q_OBJECT

public:

    explicit Widget(QWidget *parent = 0);

    ~Widget();

private:

    Ui::Widget *ui;

    QTcpServer *server;

    QTcpSocket *receivedSocket;

    QFile *newFile;

    QByteArray inBlock;

    QByteArray inBlock_sum;

    QString fileName_current; //当前文件

    QString fileName_intial;//初始文件

    qint64 totalSize;  //总共需要发送的文件大小（文件内容&文件名信息）

    qint64 byteReceived;  //已经接收的大小

    QDir dir; //默认文件储存位置

    QImage unlockimage;//解密图片

    QImage wv_image;//小波变换图片


private slots:

    void acceptConnection();

    void readClient();

    QString getHostIpAddress();//获取本机IP地址

    void ShowFile();//显示接收到的图像信息

    void ShowPicture(QImage img);//显示图像

    QImage unlock(QString lock_name);//解密函数

    void on_pushButton_clicked();//开始监听

    void on_pushButton_4_clicked();//刷新IP

    void on_pushButton_6_clicked();//断开连接

    void on_treeWidget_doubleClicked(const QModelIndex &index);//文件夹内的图像显示

    void on_pushButton_2_clicked();//图像解密

    void on_pushButton_3_clicked();//小波变换按钮
    void on_pushButton_5_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_7_clicked();
};

#endif // WIDGET_H

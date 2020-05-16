#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QHostInfo>
#include <QIODevice>
#include <QBuffer>
#include <QtCore/qmath.h>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->progressLabel_5->setText(getHostIpAddress());
    tcpClient = new QTcpSocket(this);

    connect(tcpClient, SIGNAL(connected()), this, SLOT(send()));  //当连接成功时，就开始传送文件
    connect(tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(goOnSend(qint64)));
    connect(tcpClient,SIGNAL(connected()),this,SLOT(tcp_connected()));//tcp连接槽
    connect(tcpClient,SIGNAL(disconnected()),this,SLOT(dis_connected()));//tcp断开槽

    sendTimes = 0;
}

void Widget::send()  //发送文件头信息
{
    byteToWrite = localFile_current->size();  //剩余数据的大小
    totalSize = localFile_current->size();

    loadSize = 3*1024;  //每次发送数据的大小/字节

    QDataStream out(&outBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
//  QString currentFileName = fileName.right(fileName.size() - fileName.lastIndexOf('/')-1);

    QString currentFileName = localFile_current->fileName().right(localFile_current->fileName().size() - localFile_current->fileName().lastIndexOf('/')-1);
    currentFileName=currentFileName.replace("(副本)","");
    qDebug()<<currentFileName;
    out<<qint64(0)<<qint64(0)<<currentFileName;
    totalSize += outBlock.size();  //总大小为文件大小加上文件名等信息大小
    byteToWrite += outBlock.size();

    out.device()->seek(0);  //回到字节流起点来写好前面连个qint64，分别为总大小和文件名等信息大小
    out<<totalSize<<qint64(outBlock.size());

    tcpClient->write(outBlock);  //将读到的文件发送到套接字

    outBlock.resize(0);

    ui->sendProgressBar->setMaximum(totalSize);
    ui->sendProgressBar->setValue(totalSize - byteToWrite);

}

void Widget::goOnSend(qint64 numBytes)  //开始发送文件内容
{
    byteToWrite -= numBytes;  //剩余数据大小

    outBlock = localFile_current->read(qMin(byteToWrite, loadSize));


    //qDebug()<<outBlock;

    tcpClient->write(outBlock);

    outBlock.resize(0);
    ui->sendProgressBar->setMaximum(totalSize);
    ui->sendProgressBar->setValue(totalSize - byteToWrite);

    if(byteToWrite == 0)  //发送完毕
       {
        localFile_current->remove();//删除副本
        ui->sendStatusLabel->setText(tr("文件发送完毕!"));

        loadSize = 0;
        byteToWrite = 0;
        totalSize = 0;
        outBlock.clear();
       }
}

Widget::~Widget()
{
  delete ui;
  delete tcpClient;
  if(localFile_current->isOpen())localFile_current->remove();
}

QImage Widget::toGray( QImage image )//转化为灰度图像
{
    int height = image.height();
    int width = image.width();
    QImage ret(width, height, QImage::Format_Indexed8);

    ret.setColorCount(256);    for(int i = 0; i < 256; i++)
    {
        ret.setColor(i, qRgb(i, i, i));
    }
    switch(image.format())
    {
    case QImage::Format_Indexed8: for(int i = 0; i < height; i ++)
        {   const uchar *pSrc = (uchar *)image.constScanLine(i);
            uchar *pDest = (uchar *)ret.scanLine(i);
            memcpy(pDest, pSrc, width);
        }   break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied: for(int i = 0; i < height; i ++)
        {
            const QRgb *pSrc = (QRgb *)image.constScanLine(i);
            uchar *pDest = (uchar *)ret.scanLine(i);
            for( int j = 0; j < width; j ++)
            {
                pDest[j] = qGray(pSrc[j]);
            }
        }        break;
    }

    return ret;
}

QImage Widget::locked( QString lock_name )//加密函数
{
           QImage image(lock_name);
           int chaos;
           double a;
           double x;
           double k1=0.8;
           double keys[4][2]={{4, 0.6}, {3.999, 0.8}, {3.7, 0.7}, {3.888, 0.6}} ;
//           keys[0][0]=k1/2+3.50;
//           keys[1][0]=qLn(k1+1.1)+3.5;
//           keys[2][0]=qLn(k1+1.3)+3.51;
//           keys[3][0]=qLn(k1+1.2)+3.55;
//           keys[0][1]=k1/2;
//           keys[1][1]=qLn(k1+1);
//           keys[2][1]=qLn(k1+1.3);
//           keys[3][1]=qLn(k1+1.2);

           for (int kNum = 0; kNum < 4; ++kNum)
           {
               a = keys[kNum][0];
               x = keys[kNum][1];
               for (int c = 0; c < 1000; ++c)
                   x = a*x*(1-x);

               for (int j = 0; j < image.height(); ++j)
               {
                   for (int i = 0; i < image.width(); ++i)
                   {
                       chaos = 0;
                       for (int c = 0; c < 8; ++c)
                       {
                           x = a*x*(1-x);
                           if (x >= a/6)
                               chaos = ((chaos << 1) | 1);
                           else
                               chaos = ((chaos << 1) | 0);
                       }
                       QColor color(image.pixel(i, j));
                       color.setRed(color.red() ^ chaos);
                       color.setGreen(color.green() ^ chaos);
                       color.setBlue(color.blue() ^ chaos);
                       image.setPixel(i, j, color.rgb());
                   }
               }
           }

//           QString saveName = QFileDialog::getSaveFileName(0, "保存文件", "", "");
//           image.save(saveName, fileName.section('.', -1).toStdString().c_str(), 100);
            return image;
}

QString Widget::getHostIpAddress()//获取IP地址
{
    //获取本机IP
        QString ipAddr;
        QString localHost = QHostInfo::localHostName();
        QHostInfo info = QHostInfo::fromName(localHost);

        info.addresses();//QHostInfo的address函数获取本机ip地址
        //如果存在多条ip地址ipv4和ipv6：
        foreach(QHostAddress address,info.addresses())
        {
            if(address.protocol()==QAbstractSocket::IPv4Protocol){//只取ipv4协议的地址
                qDebug()<<address.toString();
                ipAddr = address.toString();
            }
        }
    return ipAddr;
}

void Widget::tcp_connected()//tcp连接成功，提示成功信息
{
    QString str="Ip:   ";
    str.append(ui->ipEdit->toPlainText());
    str.append("\n");
    str.append("\nPort:   ");
    str.append(ui->portEdit->toPlainText());
    str.append("\n");
    str.append("\nConnected! Sending the file...  ");
    QMessageBox::information(NULL, "提示",str,QMessageBox::Yes,QMessageBox::Yes);

    isconnected == true;
}

void Widget::dis_connected()//tcp连接断开
{
    QMessageBox::information(NULL, "提示","Tcp connect abort",QMessageBox::Yes,QMessageBox::Yes);
    isconnected = false;

    sendTimes = 0;
    tcpClient = new QTcpSocket(this);//创建tcpSocket
    connect(tcpClient, SIGNAL(connected()), this, SLOT(send()));  //当连接成功时，就开始传送文件
    connect(tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(goOnSend(qint64)));
    connect(tcpClient,SIGNAL(connected()),this,SLOT(tcp_connected()));//tcp连接槽
    connect(tcpClient,SIGNAL(disconnected()),this,SLOT(dis_connected()));//tcp断开槽
}

void Widget::on_pushButton_clicked()//打开文件并获取文件名按钮
{
    ui->sendStatusLabel->setText(tr("正在打开图像..."));
    ui->sendProgressBar->setValue(0);
    loadSize = 0;
    byteToWrite = 0;
    totalSize = 0;
    outBlock.clear();

    fileName = QFileDialog::getOpenFileName(this);

    localFile = new QFile(fileName);
    localFile->open(QFile::ReadOnly);

    if(!fileName.contains(".png"))//对非png图像先做格式转换处理
    {
         localFile->copy(fileName,fileName.left(fileName.size()-3)+"png(副本)");
         QImage image(fileName.left(fileName.size()-3)+"png(副本)");
         image.save(fileName.left(fileName.size()-3)+"png(副本)","png",100);
         localFile_current = new QFile(fileName.left(fileName.size()-3)+"png(副本)");//创建传输图片的副本，以便进行加密编码操作
         localFile_current->open(QFile::ReadOnly);
    }
    else
    {
        localFile->copy(fileName,fileName+"(副本)");
        localFile_current = new QFile(fileName+"(副本)");//创建传输图片的副本，以便进行加密编码操作
        localFile_current->open(QFile::ReadOnly);
    }

    if(fileName.contains(".jpg")  || fileName .contains(".bmp")  || fileName.contains(".png")||
       fileName.contains(".gif")  || fileName.contains(".ico")  || fileName.contains(".jpeg"))
    {//确保打开的文件为图片
    QImage Image;
    Image.load(fileName);

    QPixmap pixmap = QPixmap::fromImage(Image);
    int with = ui->pictureLabel->width();
    int height = ui->pictureLabel->height();
    //QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
    QPixmap fitpixmap = pixmap.scaled(with, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 按比例缩放
    ui->sendStatusLabel->setText(tr("已打开图像 %1").arg(fileName));
    ui->pictureLabel->setPixmap(fitpixmap);
    ui->pictureLabel->show();
    ui->pictureLabel_1->setText(tr("原图："));
    }
    else  ui->sendStatusLabel->setText(tr("请打开图像文件！"));
}

void Widget::on_pushButton_2_clicked()//发送文件按钮
{
    QString ipaddr;
    int port;
    ipaddr = ui->ipEdit->toPlainText();//IP地址获取
    port = ui->portEdit->toPlainText().toInt();//tcp端口获取

    if(fileName!=NULL&&sendTimes == 0)
    {
         tcpClient->connectToHost(ipaddr,port);//连接tcpSocket
         sendTimes = 1;
         ui->sendStatusLabel->setText(tr("正在发送文件 %1").arg(fileName));
    }else if(fileName!=NULL&&sendTimes != 0)
    {
        if(localFile_current->isOpen())
        {
            send();
            ui->sendStatusLabel->setText(tr("正在发送文件 %1").arg(fileName));
        }else if(!localFile_current->isOpen())QMessageBox::information(NULL, "提示","文件已发送！",QMessageBox::Yes,QMessageBox::Yes);

    }
    else if(fileName==NULL) QMessageBox::information(NULL, "提示","Please Open the file frist!",QMessageBox::Yes,QMessageBox::Yes);
    else if(isconnected==false) QMessageBox::information(NULL, "提示","Please Link to others frist!",QMessageBox::Yes,QMessageBox::Yes);
}

void Widget::on_pushButton_7_clicked()//刷新IP地址
{
      ui->progressLabel_5->setText(getHostIpAddress());
}

void Widget::on_pushButton_8_clicked()//转灰度图像按钮
{
    if(fileName!=NULL){
    QImage Image;
    Image.load(fileName);
    Image = toGray(Image);//转灰度图
    QPixmap pixmap = QPixmap::fromImage(Image);

    QBuffer buffer;//图转bit
    buffer.open(QIODevice::ReadWrite);
    pixmap.save(&buffer,"jpg");
    QByteArray pixArray;
    pixArray.append(buffer.data());

    localFile_current->close();//更新当前文件
    localFile_current->open(QFile::WriteOnly);
    localFile_current->write(pixArray);
    localFile_current->close();
    localFile_current->open(QFile::ReadOnly);

    int with = ui->pictureLabel->width();
    int height = ui->pictureLabel->height();
    //QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
    QPixmap fitpixmap = pixmap.scaled(with, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 按比例缩放
    ui->pictureLabel->setPixmap(fitpixmap);
    ui->pictureLabel->show();
    ui->pictureLabel_1->setText(tr("灰度图："));
    }
    else QMessageBox::information(NULL, "提示","Please Open the file frist!",QMessageBox::Yes,QMessageBox::Yes);

}

void Widget::on_pushButton_9_clicked()//显示原图
{
    if(fileName!=NULL){
    QImage Image;
    Image.load(fileName);
    QPixmap pixmap = QPixmap::fromImage(Image);

    QBuffer buffer;//图转bit
    buffer.open(QIODevice::ReadWrite);
    pixmap.save(&buffer,"jpg");
    QByteArray pixArray;
    pixArray.append(buffer.data());

    localFile_current->close();//更新当前文件
    localFile_current->open(QFile::WriteOnly);
    localFile_current->write(pixArray);
    localFile_current->close();
    localFile_current->open(QFile::ReadOnly);

    int with = ui->pictureLabel->width();
    int height = ui->pictureLabel->height();
    //QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
    QPixmap fitpixmap = pixmap.scaled(with, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 按比例缩放
    ui->pictureLabel->setPixmap(fitpixmap);
    ui->pictureLabel->show();
    ui->pictureLabel_1->setText(tr("原图："));
    }
    else QMessageBox::information(NULL, "提示","Please Open the file frist!",QMessageBox::Yes,QMessageBox::Yes);
}

void Widget::on_pushButton_3_clicked()//加密按钮
{
     if(localFile_current->fileName()!=NULL){
         ui->pushButton_6->setEnabled(true);

         QImage locked_image=locked(localFile_current->fileName());

         QString str= localFile_current->fileName().right(localFile_current->fileName().size() - localFile_current->fileName().lastIndexOf('/')-1);
         str=str.replace("(副本)","");

         if(str.right(3)=="png")
         {
              locked_image.save(localFile_current->fileName(),"png",100);

         }else locked_image.save(localFile_current->fileName(),localFile->fileName().section('.', -1).toStdString().c_str(),100);

//         localFile_current->open(QFile::WriteOnly|QFile::Append);
//         QByteArray Block=12;
//         localFile_current->write(Block);
//         localFile_current->close();

         QPixmap pixmap = QPixmap::fromImage(locked_image);
         int with = ui->pictureLabel->width();
         int height = ui->pictureLabel->height();
         //QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
         QPixmap fitpixmap = pixmap.scaled(with, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 按比例缩放
         ui->pictureLabel->setPixmap(fitpixmap);
         ui->pictureLabel->show();
         ui->pictureLabel_1->setText(tr("加密后图像："));
         //加密算法

     }
     else QMessageBox::information(NULL, "提示","Please Open the file frist!",QMessageBox::Yes,QMessageBox::Yes);


}

void Widget::on_pushButton_6_clicked()//解密按钮
{
    QImage Image;
    Image.load(fileName);

    QPixmap pixmap = QPixmap::fromImage(Image);
    int with = ui->pictureLabel->width();
    int height = ui->pictureLabel->height();
    //QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
    QPixmap fitpixmap = pixmap.scaled(with, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 按比例缩放
    ui->pictureLabel->setPixmap(fitpixmap);
    ui->pictureLabel->show();
    ui->pictureLabel_1->setText(tr("原图："));


    localFile_current->remove();

    localFile = new QFile(fileName);
    localFile->open(QFile::ReadOnly);
    localFile->copy(fileName,fileName+"(副本)");
    localFile_current = new QFile(fileName+"(副本)");//创建传输图片的副本，以便进行加密编码操作
    localFile_current->open(QFile::ReadOnly);
}



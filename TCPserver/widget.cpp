#include "widget.h"
#include "ui_widget.h"
#include <QHostInfo>
#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QImageReader>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>
#include <nonoise_rgb_wv.cpp>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->pushButton_6->setEnabled(false);
    ui->label_3->setText(getHostIpAddress());
    ShowFile();
}

void Widget::acceptConnection()//tcp连接成功
{
    ui->receivedStatusLabel->setText(tr("已连接，正在准备接收文件!"));
    ui->pushButton_6->setEnabled(true);
    receivedSocket = server->nextPendingConnection();
    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(receivedSocket, SIGNAL(disconnected()), this, SLOT(on_pushButton_6_clicked()));
    server->close();//不再监听，接收数据
}

void Widget::readClient()//接收文件
{
    ui->receivedStatusLabel->setText(tr("正在接收文件..."));
    if(byteReceived == 0)  //才刚开始接收数据，此数据为文件信息
    {
        ui->receivedProgressBar->setValue(0);
        QDataStream in(receivedSocket);
        in.setVersion(QDataStream::Qt_5_0); //设置流版本，以防数据格式错误
        in>>totalSize>>byteReceived>>fileName_current;
    }
    else  //正式读取文件内容
    {
        inBlock = receivedSocket->readAll();
        byteReceived +=inBlock.size();
        inBlock_sum.append(inBlock);
    }
    ui->progressLabel->show();
    ui->receivedProgressBar->setMaximum(totalSize);
    ui->receivedProgressBar->setValue(byteReceived);
    if(byteReceived == totalSize)//完成接收
    {
        fileName_current = dir.dirName() + "/(加密)" + fileName_current;
        newFile = new QFile(fileName_current);//保存图片
        newFile->open(QFile::WriteOnly);
        newFile->write(inBlock_sum);
        newFile->close();
        QImage Image;
        Image.load(fileName_current);
        ShowPicture(Image);

        ShowFile();//显示文件信息
        ShowPicture(Image);//显示图像

        qDebug()<<Image.depth();

        ui->receivedStatusLabel->setText(tr("%1接收完成").arg(fileName_current));
        inBlock.clear();
        inBlock_sum.clear();
        byteReceived = 0;
        totalSize = 0;
        qDebug()<<"读取文件完成";
    }
}

Widget::~Widget()
{
    delete ui;
}

void Widget::ShowFile()//显示接收到的文件信息
{
     dir = QDir("Received_Picture");
     dir.setFilter(QDir::Files|QDir::Hidden|QDir::NoSymLinks);
     dir.setSorting(QDir::Time);
     QFileInfoList list = dir.entryInfoList();

     ui->treeWidget->clear();

     for (int i = 0; i < list.size(); ++i)
     {
         QFileInfo fileInfo = list.at(i);
         QTreeWidgetItem *newItem=new QTreeWidgetItem();
         newItem->setText(0,fileInfo.fileName());
         newItem->setText(1,QString::number(fileInfo.size()));
         newItem->setText(2,fileInfo.created().toString("yyyy-MM-dd hh:mm:ss"));

         ui->treeWidget->addTopLevelItem(newItem);
     }
}

void Widget::ShowPicture(QImage img)//显示图像
{
    QPixmap pixmap = QPixmap::fromImage(img);
    int with = ui->label_5->width();
    int height = ui->label_5->height();
    //QPixmap fitpixmap = pixmap.scaled(with, height, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
    QPixmap fitpixmap = pixmap.scaled(with, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);  // 按比例缩放
    ui->label_5->setPixmap(fitpixmap);
}

QString Widget::getHostIpAddress()//获取本机活动IP
{
     QString ipAddr;
     QString localHost = QHostInfo::localHostName();
     QHostInfo info = QHostInfo::fromName(localHost);
     info.addresses();//QHostInfo的address函数获取本机ip地址
     //如果存在多条ip地址ipv4和ipv6：
     foreach(QHostAddress address,info.addresses())
     {
        if(address.protocol()==QAbstractSocket::IPv4Protocol)
          {//只取ipv4协议的地址
           //qDebug()<<address.toString();
             ipAddr = address.toString();
          }
        }
    return ipAddr;
}

QImage Widget::unlock( QString lock_name )//解密函数
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

void Widget::on_pushButton_clicked()//开始监听
{
    totalSize = 0;
    byteReceived = 0;
    int port;
    port = ui->portEdit->toPlainText().toInt();
    if (port!=0){

    server = new QTcpServer(this);

    server->listen(QHostAddress::Any, port);
    connect(server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));

    ui->receivedProgressBar->setValue(0);
    ui->receivedStatusLabel->setText(tr("Listening..."));
    }
    else ui->receivedStatusLabel->setText(tr("Please input the port"));
}

void Widget::on_pushButton_4_clicked()//刷新IP
{
     ui->label_3->setText(getHostIpAddress());
}

void Widget::on_pushButton_6_clicked()//断开连接
{
    receivedSocket->abort();
    ui->receivedStatusLabel->setText(tr("已断开连接"));
    ui->pushButton_6->setEnabled(false);

    inBlock.clear();
    inBlock_sum.clear();
    byteReceived = 0;
    totalSize = 0;
}

void Widget::on_treeWidget_doubleClicked(const QModelIndex &index)//文件夹内的图像显示
{
    QString name = ui->treeWidget->currentItem()->text(0);
    name = dir.dirName() + "/" + name ;
    fileName_current=name;
    QImage Image;
    Image.load(fileName_current);
    ShowPicture(Image);
}

void Widget::on_pushButton_8_clicked()//刷新文件夹显示
{
    ShowFile();
}

void Widget::on_pushButton_2_clicked()//图像解密
{
    if(ui->treeWidget->currentItem())
    {
        fileName_current = dir.dirName() + "/" + ui->treeWidget->currentItem()->text(0);
    }

    if(fileName_current!=NULL&&fileName_current.mid(dir.dirName().size()+1, 4)=="(加密)")
    {
//        QFile *File = new QFile(fileName_current);
//        File->open(QFile::ReadOnly);
//        File->seek(File->size()-3);
//        QByteArray Block = File->read(3);
//        File->close();
//        qDebug()<<Block;

        unlockimage=unlock(fileName_current);//图像解密

        ShowPicture(unlockimage);//显示图像
    }
    else ui->receivedStatusLabel->setText("请选择未解密的图片！");

}

void Widget::on_pushButton_3_clicked()//图像小波变换按钮
{
    if(fileName_current.mid(dir.dirName().size()+1, 4)!="(加密)"&&fileName_current!=NULL)
    {
      QMessageBox::information(NULL, "提示","此操作可能花费数分钟的时间\n请耐心等待\n点击“Yes”开始！",QMessageBox::Yes,QMessageBox::Yes);
      wv_image.load(fileName_current);
      wv_image=toGray(wv_image);//小波变换
      ui->receivedStatusLabel->setText("图像优化已完成！");
      ShowPicture(wv_image);//显示图像
    }
    else ui->receivedStatusLabel->setText("请选择未加密的图片！");
}

void Widget::on_pushButton_7_clicked()//保存小波变换图像
{
    if(fileName_current.mid(dir.dirName().size()+1, 4)!="(加密)")
    {
        wv_image.save(fileName_current,fileName_current.section('.', -1).toStdString().c_str(), 100);
        ui->receivedStatusLabel->setText("小波变换图片已保存！");
        ShowFile();
    }
}

void Widget::on_pushButton_5_clicked()//保存解密图像
{
    QString str=dir.dirName()+ "/" + fileName_current.mid(dir.dirName().size()+5);
    unlockimage.save(str,fileName_current.section('.', -1).toStdString().c_str(), 100);
    ui->receivedStatusLabel->setText("解密图片已保存！");
    ShowFile();
}








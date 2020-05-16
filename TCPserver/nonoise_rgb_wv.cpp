#include <iostream>
#include <vector>
#include <QImage>
#include <qimage.h>
#include <qstring.h>
#include <qpainter.h>
#include <cwavelet.h>
#include <cwavelet_def.h>
#include <random>

using namespace std;
using namespace wavelet;

class mat{
public:
    mat(){}
    mat(QImage &img):width(img.width()),
    height(img.height()),_img(img){}
    //构造函数，通过qt QImage类图片来完成mat类的初始化
    void filldata();
    //将QImage类图片内的数据转成灰度数据并附给mat类成员
public:
    int width;
    int height;
    QImage _img;
    std::vector<double> data1;
    std::vector<double> data2;
    std::vector<double> data3;
};

void mat::filldata()//形成一维灰度数组
{
    for(int i=0;i<height;i++)
    { for(int j=0;j<width;j++)
        {
          data1.push_back(qRed(_img.pixel(j,i)));//R

        }
    }
    for(int i=0;i<height;i++)
    { for(int j=0;j<width;j++)
        {
          data2.push_back(qGreen(_img.pixel(j,i)));//G

        }
    }
    for(int i=0;i<height;i++)
    { for(int j=0;j<width;j++)
        {
          data3.push_back(qBlue(_img.pixel(j,i)));//B

        }
    }
}
QImage toGray(QImage imag)
{
//    std::default_random_engine e; //引擎
//    std::normal_distribution<double> n(0, 50); //均值, 方差

    mat image(imag);
    image.filldata();
    int s_size;
    s_size=image.height*image.width;
    double*s1;
    double*s2;
    double*s3;
    s1=new double[s_size];
    s2=new double[s_size];
    s3=new double[s_size];
    for(int i=0;i<s_size;i++)
    {
//        s1[i]=image.data1[i]+n(e);//R
        s1[i]=image.data1[i];
     }
    for(int i=0;i<s_size;i++)
    {
//        s2[i]=image.data2[i]+n(e);//G
        s2[i]=image.data2[i];
     }
    for(int i=0;i<s_size;i++)
    {
//        s3[i]=image.data3[i]+n(e);//B
        s3[i]=image.data3[i];
     }
//以上为预处理，从输入的QImage彩图(或灰度图)，得到小波变换所需参数




//以下实现小波阈值去噪
    system("color 0A");
    CWavelet cw;
    int Scale = 3;
    int dbn = 3;
    cw.InitDecInfo2D(image.height, image.width, Scale, dbn);
    double *dstcoef1= new double[s_size];//R
    double *dstcoef2= new double[s_size];//G
    double *dstcoef3= new double[s_size];//B
    if (!cw.thrDenoise2D(s1, dstcoef1))
        cerr << "Error" << endl;
    if (!cw.thrDenoise2D(s2, dstcoef2))
        cerr << "Error" << endl;
    if (!cw.thrDenoise2D(s3, dstcoef3))
        cerr << "Error" << endl;
//以上实现小波阈值去噪




//以下收尾处理，将小波变换所得信息转为QImage类型
    QImage newImage = QImage(image.width, image.height, QImage::Format_ARGB32);
    for(int y = 0; y<newImage.height(); y++)//行
    {
        for(int x = 0; x<newImage.width(); x++)
        {
                //double average=dstcoef[y*image.width+x];//此处应是处理后的数组dstcoef

                newImage.setPixel(x,y, qRgb(dstcoef1[y*image.width+x],dstcoef2[y*image.width+x],dstcoef3[y*image.width+x]));
         }

     }
    delete[] dstcoef1;
    delete[] s1;
    delete[] dstcoef2;
    delete[] s2;
    delete[] dstcoef3;
    delete[] s3;
    return newImage;//以QImage类型输出灰度图
}

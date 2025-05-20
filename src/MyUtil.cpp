#include "MyUtil.h"
#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <unistd.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h> 
//#include <direct.h>
//#include <io.h>
using namespace std;

namespace kmod {

    
    cv::Mat MyUtil::QImage2cvMat(const QImage& image)
    {
        switch (image.format())
        {
            // 8-bit, 4 channel
        case QImage::Format_ARGB32:
            break;
        case QImage::Format_ARGB32_Premultiplied:
        {
            cv::Mat mat(image.height(), image.width(),
                CV_8UC4,
                (void*)image.constBits(),
                image.bytesPerLine());
            return mat.clone();
        }

        // 8-bit, 3 channel
        case QImage::Format_RGB32:
        {
            cv::Mat mat(image.height(), image.width(),
                CV_8UC4,
                (void*)image.constBits(),
                image.bytesPerLine());

            // drop the all-white alpha channel
            cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR);
            return mat.clone();
        }
        case QImage::Format_RGB888:
        {
            QImage swapped = image.rgbSwapped();
            cv::Mat mat(swapped.height(), swapped.width(),
                CV_8UC3,
                (void*)image.constBits(),
                image.bytesPerLine());
            return mat.clone();
        }

        // 8-bit, 1 channel   
        case QImage::Format_Indexed8:
        {
            cv::Mat mat(image.height(), image.width(),
                CV_8UC1,
                (void*)image.constBits(),
                image.bytesPerLine());
            return mat.clone();
        }

        // wrong
        default:
           // qDebug() << "ERROR: QImage could not be converted to Mat.";
            break;
        }
        return cv::Mat();
    }
    QImage MyUtil::MatToQImage(const cv::Mat& mat)
    {
        //qDebug() << "qimagecreateL:"<< mat.type();
        switch (mat.type())
        {
            // 8-bit, 4 channel
        case CV_8UC4:
        {
           // std::cout << "qimagecreateL:32";
           /* QImage image(mat.data,
                mat.cols, mat.rows,
                static_cast<int>(mat.step),
                QImage::Format_ARGB32);*/

            const unsigned char* bytes = mat.data;
            QImage image = QImage(bytes, mat.cols, mat.rows,
                static_cast<int>(mat.step), QImage::Format_ARGB32).copy();
            return image;
        }

        // 8-bit, 3 channel
        case CV_8UC3:
        {
           // std::cout << "qimagecreateL:24";
           /* QImage image(mat.data,
                mat.cols, mat.rows,
                static_cast<int>(mat.step),
                QImage::Format_RGB888).copy();*/

            const unsigned char* bytes = mat.data;
            QImage image = QImage(bytes, mat.cols, mat.rows,
                static_cast<int>(mat.step), QImage::Format_RGB888).copy();
            return image.rgbSwapped();
        }

        // 8-bit, 1 channel
        case CV_8UC1:
        {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
           
            const unsigned char* bytes = mat.data;
            QImage image = QImage(bytes, mat.cols, mat.rows,
                static_cast<int>(mat.step), QImage::Format_Grayscale8).copy();
           
                
#else
            static QVector<QRgb>  sColorTable;

            // only create our color table the first time
            if (sColorTable.isEmpty())
            {
                sColorTable.resize(256);

                for (int i = 0; i < 256; ++i)
                {
                    sColorTable[i] = qRgb(i, i, i);
                }
            }

            QImage image(mat.data,
                mat.cols, mat.rows,
                static_cast<int>(mat.step),
                QImage::Format_Indexed8);

            image.setColorTable(sColorTable);
#endif

            return image;
        }

        // wrong 
        default:
           // qDebug() << "ERROR: Mat could not be converted to QImage.";
            break;
        }
        return QImage();
    }

   int64_t MyUtil::GetTimeStamp(){
        int64_t times = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch()).count();
        return times;
   }

   bool MyUtil::pathExist(std::string sFullPath){
   
      

       size_t pos = sFullPath.rfind("/");
       if (pos + 1 != sFullPath.size()) {
           sFullPath += "/";     
       }
       size_t index = sFullPath.find("/", 0);
       if (index > sFullPath.length()) return false;
       while ((index = sFullPath.find("/", index + 1)) < sFullPath.length()){          
           string tmpPath = sFullPath.substr(0, index);
           if (access(tmpPath.c_str(), 0) != 0){ 
               if (mkdir(tmpPath.c_str(),0777) != 0) {
                   return false;
               }
           }

       }
       return true;
   }

    std::string MyUtil::getExePath()
    {

        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        std::string exePath = std::string(result, (count > 0) ? count : 0);
        size_t pos = exePath.find_last_of("\\/");
        return (std::string::npos == pos)
            ? ""
            : exePath.substr(0, pos);
    }   
   std::tm* MyUtil::Gettm(long long timestamp)
   {

       auto mTime = std::chrono::milliseconds(timestamp);
       auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
       auto tt = std::chrono::system_clock::to_time_t(tp);
       // std::tm* now  = gmtime(&stTime);
       std::tm* now = localtime(&tt);
       return now;
   }

   std::string MyUtil::Gettmstr(long long timestamp)
   {
       std::string  ret;
       char buf[128] = { 0 };
       auto mTime = std::chrono::milliseconds(timestamp);
       auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
       auto tt = std::chrono::system_clock::to_time_t(tp);
       //std::tm* now = gmtime(&tt);
       int ms = timestamp % 1000;
       std::tm* now = localtime(&tt);
       snprintf(buf,128,"%d/%d/%d_%d:%d:%d,%d", now->tm_year + 1900, now->tm_mon + 1, 
           now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, ms);
       
       ret = buf;
       return ret;
   }
   std::string MyUtil::GettmstrFile(long long timestamp)
   {
       std::string  ret;
       char buf[128] = { 0 };
       auto mTime = std::chrono::milliseconds(timestamp);
       auto tp = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>(mTime);
       auto tt = std::chrono::system_clock::to_time_t(tp);
       //std::tm* now = gmtime(&tt);
       int ms = timestamp % 1000;
       std::tm* now = localtime(&tt);
       snprintf(buf, 128, "%d-%d-%d_%d-%d-%d_%d", now->tm_year + 1900, now->tm_mon + 1,
           now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec, ms);

       ret = buf;
       return ret;
   }

  cv::Mat MyUtil::getDefaultMat() {

       cv::Mat _defaultmat = cv::Mat(1024, 1280, CV_8UC1, cv::Scalar(0));
       int fontface = cv::FONT_HERSHEY_DUPLEX;
       double fontscale = 5.0;
       int fontthick = 5;
       int baseline = 0;
       std::string txtout = "NO IMAGES";
       cv::Size txtsize = cv::getTextSize(txtout, fontface, fontscale, fontthick, &baseline);
       cv::Point txtpoint;
       txtpoint.x = _defaultmat.cols / 2 - txtsize.width / 2;
       txtpoint.y = _defaultmat.rows / 2 - txtsize.height / 2;
       cv::putText(_defaultmat, txtout, txtpoint, fontface, fontscale, cv::Scalar(150), fontthick);

       fontface = cv::FONT_HERSHEY_PLAIN;
       fontscale = 1.0;
       fontthick = 1;
       baseline = 0;
       srand(time(NULL));
       int nm = rand();
       cv::putText(_defaultmat, "code:" + std::to_string(nm), cv::Point(_defaultmat.cols / 2, 20), fontface, fontscale, cv::Scalar(255), fontthick);

       return _defaultmat;
   }
  
}

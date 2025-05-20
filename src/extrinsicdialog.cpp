#include "extrinsicdialog.h"
#include "ui_extrinsicdialog.h"
#include "calibratewidget.h"
#include "cameraparameter.h"
#include "aspectsinglelayout.h"
#include <QRect>
#include <QPoint>
#include <QPainter>
#include <QResizeEvent>
#include <QFileDialog>
#include "MyUtil.h"
using namespace kmod;


ExtrinsicDialog::ExtrinsicDialog(cv::Mat &image, std::shared_ptr<kmod::CameraParameter>  p, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExtrinsicDialog),
    camPara(p)
{
    ui->setupUi(this);
    ui->imageLabel->setAlignment(Qt::AlignLeft|Qt::AlignTop);
    image.copyTo(srcImage);
    AspectSingleLayout *aspLayout =  new AspectSingleLayout(NULL, camPara->imageWidthToHeight);
    ui->gridLayout->addLayout(aspLayout, 0, 0);
    aspLayout->setContentsMargins(0, 0, 0, 0);
    aspLayout->addWidget(ui->imageLabel);
    aspLayout->setAlignment(ui->imageLabel, Qt::AlignCenter);
    QImage qImg = kmod::MyUtil::MatToQImage(srcImage);
    ui->imageLabel->setPixmap(QPixmap::fromImage(qImg));
    ui->imageLabel->setScaledContents(true);
    connect(ui->imageLabel, &ExtrinsicLabel::mouseReleased, this, &ExtrinsicDialog::findChessboardCorners);
}

ExtrinsicDialog::~ExtrinsicDialog()
{
    delete ui;
}

void ExtrinsicDialog::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    if(isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void ExtrinsicDialog::findChessboardCorners(const QPoint &origin, const QPoint &end)
{
    if(camPara->extrinsicReady)
        return; 
    
    qInfo() << "origin:"<<origin;
    qInfo() << "end:"<<end;
        

    QPoint endBoundp = end;
    QSize currentSize = ui->imageLabel->size();
    float scaleFactor = (float)(camPara->imgSize.width) / currentSize.width();

    endBoundp.rx() = qBound(0, endBoundp.x(), currentSize.width()-1);
    endBoundp.ry() = qBound(0, endBoundp.y(), currentSize.height()-1);

    QRect qRectMask = QRect(origin * scaleFactor, endBoundp * scaleFactor).normalized();

    QPoint qorigin = qRectMask.topLeft();
    QSize qsize = qRectMask.size();
    if(qsize.width() < 20 || qsize.height() < 20) {
         qInfo() << "too small scanning area!";
         return;
    }
    
    cv::Point2f biasMain(qorigin.x(), qorigin.y());
    cv::Size mainRoiSize(qsize.width(), qsize.height());
    cv::Rect cvRectMask(biasMain, mainRoiSize);

    cv::Mat cvSrcImg;
    srcImage.copyTo(cvSrcImg);
    cv::Mat mainRoiImg = cvSrcImg(cvRectMask);

    cv::Size halfRoiSize(mainRoiSize.width / 2, mainRoiSize.height);
    cv::Point2f biasRight(mainRoiSize.width / 2, 0);
    cv::Rect cvRectLeft(cv::Point(0,0), halfRoiSize);
    cv::Rect cvRectRight(biasRight, halfRoiSize);

    cv::Mat leftRoiImg = mainRoiImg(cvRectLeft);
    cv::Mat rightRoiImg = mainRoiImg(cvRectRight);

    vector<cv::Mat> subChessBoardImgs;
    subChessBoardImgs.push_back(leftRoiImg);
    subChessBoardImgs.push_back(rightRoiImg);

    cv::imwrite("left.jpg",leftRoiImg);
    cv::imwrite("right.jpg",rightRoiImg);

    vector<cv::Point2f> originPoints;
    originPoints.push_back(cv::Point(0.0, 0.0));
    originPoints.push_back(cv::Point(halfRoiSize.width, 0.0));

    for(size_t i = 0; i < subChessBoardImgs.size(); ++i) {

        vector<cv::Point2f> corners;
        bool cfound =false ;
        cv::Point2f far, near;

        cfound = cv::findChessboardCorners(subChessBoardImgs[i], cv::Size(3, 3), corners);

        if(!cfound) {
            qInfo() <<i << "corners not found!";
            subCorners.clear();
            return;
        }else{
             qInfo() <<i << "corners  found!";
        }
        cv::Mat grayImage;
        cv::cvtColor(subChessBoardImgs[i], grayImage, cv::COLOR_BGR2GRAY);
        cv::cornerSubPix(grayImage, corners, cv::Size(5, 5),
                         cv::Size(-1, -1),
                         cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 30, 0.1));

        far = corners.at(0);
        near = corners.at(0);
        float resFar, resNear;
        resFar = resNear = cv::norm(far - originPoints[i]);
        for(size_t j = 1; j < corners.size(); ++j) {
            float res = cv::norm(corners[j] - originPoints[i]);
            if(res < resNear) {
                near = corners[j];
                resNear = res;
            } else if(res > resFar) {
                far = corners[j];
                resFar = res;
            }
        }
        subCorners.push_back(far);
        subCorners.push_back(near);
    }

    for(size_t i = 2; i < subCorners.size(); ++i) {
        subCorners[i] += biasRight;
    }

    for(size_t i = 0; i < subCorners.size(); ++i) {
        subCorners[i] += biasMain;
        cv::circle(cvSrcImg, subCorners[i], 5, cv::Scalar(140, 155, 190), 2);
    }

    QImage imgShow = kmod::MyUtil::MatToQImage(cvSrcImg);
    ui->imageLabel->setPixmap(QPixmap::fromImage(imgShow));

   ;// makeExtrinsic();
}

void ExtrinsicDialog::makeExtrinsic()
{
    vector<cv::Point3f> objPoints;

    /* to be changed */
    if(camPara->name == "front") {
        objPoints.push_back(cv::Point3f(-7.0, 12.0, 0.0));
        objPoints.push_back(cv::Point3f(-11.0, 16.0, 0.0));
        objPoints.push_back(cv::Point3f(7.0, 12.0, 0.0));
        objPoints.push_back(cv::Point3f(11.0, 16.0, 0.0));
    } else if(camPara->name == "left") {
        objPoints.push_back(cv::Point3f(-7.0, -12.0, 0.0));
        objPoints.push_back(cv::Point3f(-11.0, -16.0, 0.0));
        objPoints.push_back(cv::Point3f(-7.0, 12.0, 0.0));
        objPoints.push_back(cv::Point3f(-11.0, 16.0, 0.0));
    } else if(camPara->name == "back") {
        objPoints.push_back(cv::Point3f(7.0, -12.0, 0.0));
        objPoints.push_back(cv::Point3f(11.0, -16.0, 0.0));
        objPoints.push_back(cv::Point3f(-7.0, -12.0, 0.0));
        objPoints.push_back(cv::Point3f(-11.0, -16.0, 0.0));
    } else {
        objPoints.push_back(cv::Point3f(7.0, 12.0, 0.0));
        objPoints.push_back(cv::Point3f(11.0, 16.0, 0.0));
        objPoints.push_back(cv::Point3f(7.0, -12.0, 0.0));
        objPoints.push_back(cv::Point3f(11.0, -16.0, 0.0));
    }

    float alpha = 1.0/36;

    cv::Point3f t3(0.0, 0.0, 0.0);
    for(uint i = 0; i < objPoints.size(); i++) {
        objPoints[i] = (objPoints[i] - t3)*alpha;
    }

    std::vector<cv::Point2f> undistps;
    cv::fisheye::undistortPoints(subCorners, undistps, camPara->cvIntrinsic, camPara->cvDistCoeffs);

    cv::Mat rvec, tvec, rmat, extrinsic;
    cv::solvePnP(objPoints, undistps, cv::Mat::eye(3, 3, CV_64F), cv::Mat(), rvec, tvec);
    cv::Rodrigues(rvec, rmat);
    cv::hconcat(rmat, tvec, extrinsic);
    extrinsic.copyTo(camPara->cvExtrinsic);

    camPara->extrinsicReady = true;
    camPara->changed = true;

    

}
void ExtrinsicDialog::on_loadButton_clicked()
 {

//    QSettings loaderSettings(settingsFile, QSettings::IniFormat);
//     loadDirectory = loaderSettings.value("lastDirectory").toString();
    // QFileDialog fileDialog;
    // fileDialog.setViewMode(QFileDialog::Detail);
    // //fileDialog.restoreGeometry(loaderSettings.value("lastOpenDialogGeo").toByteArray());
    // QString qSNum;
    // loadFileName = fileDialog.getOpenFileName(this, tr("Load s19 file"), loadDirectory, tr("s19 (*.s19)"));
    // loaderSettings.setValue("lastOpenDialogGeo", fileDialog.saveGeometry());
    // if (loadFileName.isNull()) {



     QString filter;
    // //应用程序所在目录
    QString fileName = QApplication::applicationDirPath() + "/main.jpg";
    // //打开多个文件使用getOpenFileNames
    QString openFile = QFileDialog::getOpenFileName(this, "0.0", fileName, "*.jpg", &filter);
    if( !openFile.isNull()){
        srcImage = cv::imread(openFile.toStdString());
        //qInfo() << openFile; 
       
        bool cfound;
        cv::Point2f far, near;
        cv::Mat cvSrcImg;
        srcImage.copyTo(cvSrcImg);

        vector<cv::Point2f> corners;
        cfound = cv::findChessboardCorners(srcImage, cv::Size(3, 3), corners);
        qInfo() << openFile<<cfound;
        if( cfound){
            QImage qImg = kmod::MyUtil::MatToQImage(cvSrcImg);
            ui->imageLabel->setPixmap(QPixmap::fromImage(qImg));
            ui->imageLabel->setScaledContents(true);
        }else{
            QImage qImg = kmod::MyUtil::MatToQImage(srcImage);
            ui->imageLabel->setPixmap(QPixmap::fromImage(qImg));
            ui->imageLabel->setScaledContents(true);
        }

    }
   

    // QFileDialog openDialog;
    // //设置对话框打开的模式
    // openDialog.setAcceptMode(QFileDialog::AcceptOpen);
    // //设置对话框的标题
    // openDialog.setWindowTitle("0.0");
    // //设置对话框的打开路径(只是路径，加上文件名失效)
    // openDialog.setDirectory(QApplication::applicationDirPath());
    // //设置对话框默认选中文件名
    // //openDialog.selectFile("/main.obj");
    // //设置对话框文件过滤器
    // openDialog.setNameFilter("*.jpg");
    // //设置文件模式，例如：已存在未存在都可选择，只能选中已存在文件，同时选中多个文件等
    // openDialog.setFileMode(QFileDialog::ExistingFile);
    // if (openDialog.exec() == QFileDialog::AcceptSave) {
    //     //打开文件的路径(包含文件名)
    //    QString openFile = openDialog.selectedFiles().first();       
    //    srcImage = cv::imread(openFile.toStdString());
    //    qInfo() << openFile; 

        // bool cfound;
        // cv::Point2f far, near;
        // cv::Mat cvSrcImg;
        // srcImage.copyTo(cvSrcImg);

        // vector<cv::Point2f> corners;
        // cfound = cv::findChessboardCorners(srcImage, cv::Size(3, 3), corners);

        // if( cfound){

        //     cv::Mat grayImage;
        //     cv::cvtColor(srcImage,grayImage, cv::COLOR_BGR2GRAY);
        //     cv::cornerSubPix(grayImage, corners, cv::Size(5, 5), cv::Size(-1, -1),
        //                         cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::COUNT, 30, 0.1));

            
        //     for(size_t i = 0; i < subCorners.size(); ++i) {
                
        //         cv::circle(cvSrcImg, corners[i], 5, cv::Scalar(140, 155, 190), 2);
        //     }
        //     QImage qImg = kmod::MyUtil::MatToQImage(cvSrcImg);
        //     ui->imageLabel->setPixmap(QPixmap::fromImage(qImg));
        //     ui->imageLabel->setScaledContents(true);
        //     qInfo() << "Found!";

        // }else{
        //     QImage qImg = kmod::MyUtil::MatToQImage(srcImage);
        //     ui->imageLabel->setPixmap(QPixmap::fromImage(qImg));
        //     ui->imageLabel->setScaledContents(true);
        //     qInfo() << "NO Found!";
        //     return;
        // }



    //    //打开文件的路径
    //    QString openPath = openDialog.directory().path();
    //    //获得文件过滤器得类型
    //    QString filter = openDialog.selectedNameFilter();
   // }

 }
void ExtrinsicDialog::on_resetButton_clicked()
{
    
    if(!camPara->extrinsicReady)
        return;
    camPara->extrinsicReady = false;
    camPara->changed = true;
    subCorners.clear();
    QImage imgShow = kmod::MyUtil::MatToQImage(srcImage);
    ui->imageLabel->setPixmap(QPixmap::fromImage(imgShow));
}

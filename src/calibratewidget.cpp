#include "calibratewidget.h"
#include "ui_calibratewidget.h"
#include "aspectsinglelayout.h"
#include <QResizeEvent>

#include "KMsg.h"
#include "MyUtil.h"
#include "Datadefine.h"

using namespace kmod;

CalibrateWidget::CalibrateWidget(std::shared_ptr<kmod::CameraParameter> p, QWidget *parent = 0) :
    QDialog(parent),
    ui(new Ui::CalibrateWidget),
    camPara(p),
    frames(0),
    showUndistort(0)
{
    ui->setupUi(this);


    //ui->calibrateButton->setEnabled(false);
   // ui->captureButton->setEnabled(false);
   

    AspectSingleLayout *aspLaout_1  =  new AspectSingleLayout(NULL, camPara->imageWidthToHeight);
    ui->gridLayout_2->addLayout(aspLaout_1, 0, 0);
    aspLaout_1->addWidget(ui->srcImageLabel);
    aspLaout_1->setAlignment(ui->srcImageLabel, Qt::AlignCenter);

    AspectSingleLayout *aspLaout_2 =  new AspectSingleLayout(NULL, camPara->imageWidthToHeight);
    ui->gridLayout_2->addLayout(aspLaout_2, 1, 0);
    aspLaout_2->addWidget(ui->dstImageLabel);
    aspLaout_2->setAlignment(ui->dstImageLabel, Qt::AlignCenter);

    camCalib = new CameraCalibrator(camPara);
    captureTimer = new QTimer(this);

    connect(captureTimer, &QTimer::timeout, this, &CalibrateWidget::cameraTimeout);
    connect(this, &CalibrateWidget::sendSrcImage, camCalib, &CameraCalibrator::findAndDrawChessboard);
    connect(camCalib, &CameraCalibrator::isChessboardFound, this, &CalibrateWidget::foundChessboard);    
    connect(ui->calibrateButton, &QPushButton::clicked, camCalib, &CameraCalibrator::calibrate);
    connect(camCalib, &CameraCalibrator::sendCalibResults, this, &CalibrateWidget::receiveCalibResults);

    if(camPara->intrinsicDistReady) {
        showUndistort = true;
        ui->resetButton->setEnabled(true);
        camPara->cvIntrinsic.copyTo(camCalib->intrinsic);
        camPara->cvDistCoeffs.copyTo(camCalib->distCoeffs);
        camCalib->initCalibMap();

    } else {

       ui->captureButton->setEnabled(true);
       showUndistort = false;
    }

    camCalib->moveToThread(&calibThread);
    connect(&calibThread, &QThread::finished, camCalib, &CameraCalibrator::deleteLater);
    calibThread.start();
    captureTimer->start(30);

}

CalibrateWidget::~CalibrateWidget()
{
    
    calibThread.quit();
    calibThread.wait();
    delete ui;
}

void CalibrateWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    if(isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
}

void CalibrateWidget::on_captureButton_clicked()
{
    if(srcImage.empty()) {
        return;
    }
    srcImage.copyTo(dstImage);
    emit sendSrcImage (&dstImage );
   // ui->captureButton->setEnabled(false);
}

void CalibrateWidget::foundChessboard(bool f)
{
   

    if(f) {

        ui->dstImageLabel->clear();
       // ui->captureButton->setEnabled(true);
        
        QImage dst =  kmod::MyUtil::MatToQImage(dstImage);;
        QSize sz= ui->dstImageLabel->size();

        ui->dstImageLabel->setPixmap( QPixmap::fromImage(dst.scaled(sz, Qt::KeepAspectRatio)));
        
       
        if(!srcImage.empty()){            
            std::string fpath = "./capture/data" + std::to_string(camPara->devId) + "/";// + std::to_string(frames) +".jpg";
            kmod::MyUtil::pathExist(fpath);
           // pathExist
            cv::imwrite(fpath + std::to_string(frames) +".png", srcImage);

        }

        if(!dstImage.empty()){
            std::string fpath = "./capture/dst_" + std::to_string(camPara->devId) + "/";// + std::to_string(frames) +".jpg";
            kmod::MyUtil::pathExist(fpath);
            cv::imwrite(fpath + std::to_string(frames) +".jpg", dstImage);
        }          

        frames++;
        updateFrameNums();
        
        if(!ui->calibrateButton->isEnabled())
            ui->calibrateButton->setEnabled(true);
        if(!ui->resetButton->isEnabled())
            ui->resetButton->setEnabled(true);

    }
}

void CalibrateWidget::receiveCalibResults(bool ok)
{
    ui->resultLine->clear();
    if(ok) {
        ui->calibrateButton->setEnabled(false);
        ui->captureButton->setEnabled(false);
        ui->saveParaButton->setEnabled(true);
        showUndistort = true;
        ui->resultLine->setText(QString("avg error: %1").arg(camCalib->totalAvgErr));

    } else {
        ui->resultLine->setText(QString("calibration Failed!"));
    }
    ui->resetButton->setEnabled(true);
}

void CalibrateWidget::on_saveParaButton_clicked()
{
    ui->saveParaButton->setEnabled(false);
    ui->resetButton->setEnabled(true);
    camCalib->intrinsic.copyTo(camPara->cvIntrinsic);
    camCalib->distCoeffs.copyTo(camPara->cvDistCoeffs);
    camPara->intrinsicDistReady = true;
    camPara->changed = true;
}

void CalibrateWidget::on_resetButton_clicked()
{
    ui->resetButton->setEnabled(false);
    ui->captureButton->setEnabled(true);
    ui->saveParaButton->setEnabled(false);
    ui->calibrateButton->setEnabled(false);

    ui->dstImageLabel->clear();
    ui->resultLine->clear();
    frames = 0;
    updateFrameNums();
    camCalib->cleanPoints();
    showUndistort = false;

    if(camPara->intrinsicDistReady) {
        camPara->changed = true;
        camPara->intrinsicDistReady = false;
    }
}
void CalibrateWidget::updateFrameNums()
{
    ui->frameLine->setText(QString("frames: %1").arg(frames));
}
void CalibrateWidget::on_srcSaveButton_clicked()
{
    if(srcImage.empty())
        return;

    std::string fname = "./capture/data" + std::to_string(camPara->devId) +".png";
    cv::imwrite(fname, srcImage);
}

void CalibrateWidget::on_dstSaveButton_clicked()
{
    if(dstImage.empty())
        return;
    std::string fname = "./capture/dst_" + std::to_string(camPara->devId) +".jpg";
    cv::imwrite(fname, dstImage);
}
void CalibrateWidget::cameraTimeout()
{
    
    if(kmod::g_pDevDatav->mCameras[camPara->devId]->isConncected()) {    
        std::shared_ptr<kmod::MatTimeStamp>  mymat = kmod::g_ImgbufList[camPara->devId].pop();
        if (mymat != nullptr && !mymat->imgmat.empty()) {            
            srcImage = mymat->imgmat.clone();
            QImage src = kmod::MyUtil::MatToQImage(srcImage);
            QSize ssz = ui->srcImageLabel->size();
            ui->srcImageLabel->setPixmap(QPixmap::fromImage(src.scaled(ssz, Qt::KeepAspectRatio)));
            
             
            //srcImage.copyTo(dstImage);
            //emit sendSrcImage (&dstImage );

            if(showUndistort) {

                camCalib->calibrateImage(srcImage, dstImage);
                QImage dst =  kmod::MyUtil::MatToQImage(dstImage);;
                QSize dsz = ui->dstImageLabel->size();
                ui->dstImageLabel->setPixmap(QPixmap::fromImage(dst.scaled(dsz, Qt::KeepAspectRatio)));

            }else{
                
                if( dstImage.empty()){
                    dstImage = srcImage.clone();
                    QImage dst =  kmod::MyUtil::MatToQImage(dstImage);;
                    QSize dsz = ui->dstImageLabel->size();
                    ui->dstImageLabel->setPixmap(QPixmap::fromImage(dst.scaled(dsz, Qt::KeepAspectRatio)));
                }
            }
            
        }
    } 

}

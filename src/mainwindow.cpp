#pragma execution_character_set("utf-8")

// #include <QtPrintSupport/QPrintDialog>
// #include <QtPrintSupport/QPrinter>
// #include <QTreeWidgetItem>
// #include <QHeaderView>
//#include <QDesktopServices>



#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QRect>
#include <QPainter>
#include <qdebug.h>
#include <QtWidgets/QWidget>
#include <QActionGroup>
#include <QWidgetAction>
#include <QMessageBox>
#include <QPalette>
#include <QTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QTextCodec>



#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv2/opencv.hpp>
#include <Util/logger.h>


#include "KConfig.h"
#include "KMsg.h"
#include "MyUtil.h"
#include "aboutdlg.h"
#include "Datadefine.h"
#include "cameraCalibrator.h"

using namespace kmod;


//if (kmod::g_pDevDatav->mCameras[nid]->isopen()) {

// cv::Mat  shwomat;
// for( int i=0; i<4; i++ ) {
//     std::shared_ptr<kmod::MatTimeStamp>  mymat = kmod::g_ImgbufList[i].pop();
//     if( mymat == nullptr){
//         continue;
//         //return kmod::MyUtil::MatToQImage(_defaultmat);
//     }
//     if( mymat->imgmat.empty()){
//         continue;
//         //return kmod::MyUtil::MatToQImage(_defaultmat);
//     }
//     if(shwomat.empty() ) {

//         shwomat = cv::Mat(mymat->imgmat.rows + 1, mymat->imgmat.cols + 1, mymat->imgmat.type());
//     }
//     cv::Mat src;
//     cv::pyrDown(mymat->imgmat,src,cv::Size(mymat->imgmat.cols/2, mymat->imgmat.rows /2));

//     cv::Rect roi_rect = cv::Rect(0, 0, src.cols, src.rows);

//     if( i ==1 ){

//         roi_rect = cv::Rect(src.cols, 0, src.cols, src.rows);
//     }
//     else if( i==2 ){

//         roi_rect = cv::Rect(0, src.rows, src.cols, src.rows);
//     }
//     else if( i==3 ){

//         roi_rect = cv::Rect(src.cols, src.rows, src.cols, src.rows);
//     }
//     else if( i==0 ){

//         roi_rect = cv::Rect(0, 0, src.cols, src.rows);
//     }

//     src.copyTo(shwomat(roi_rect));
//     src.release();
// }
// if( !shwomat.empty() ){
//     return kmod::MyUtil::MatToQImage(shwomat);
// }else{
//     _defaultmat = kmod::MyUtil::getDefaultMat();
//     return kmod::MyUtil::MatToQImage(_defaultmat);
// }

class CGetOutImage {
public:
    CGetOutImage() {
        _defaultmat = cv::Mat(_nHeight, _nWidth, CV_8UC1, cv::Scalar(0));
        _cmrSum = kmod::limitV<int>( kmod::KConfig::Instance().cfg()->get<int>("camerasum"), 5, 1 );
        //for(const std::shared_ptr<kmod::CameraParameter> it: kmod::g_camerasParam){
        int nid=0;
        for( int i=0; i<_cmrSum; i++) {
            if( kmod::g_camerasParam[i]->intrinsicDistReady) {
                std::shared_ptr<CameraCalibrator> camCalib = std::make_shared<CameraCalibrator>(kmod::g_camerasParam[i]);
                kmod::g_camerasParam[i]->cvIntrinsic.copyTo(camCalib->intrinsic);
                kmod::g_camerasParam[i]->cvDistCoeffs.copyTo(camCalib->distCoeffs);
                kmod::g_camerasParam[i]->cvHomo.copyTo(camCalib->Homo);
                kmod::g_camerasParam[i]->cvmask.copyTo(camCalib->mask);
                camCalib->scale = kmod::g_camerasParam[i]->fscale;
                camCalib->useCalibrate = true;
                camCalib->initCalibMap();
                camCalibs.emplace_back(camCalib);

            }

        }
    }
    ~CGetOutImage() {

    }
    cv::Mat getNextFrame( int nid = 0) {

        if ( nid >=0 && nid <_cmrSum  && kmod::g_pDevDatav->mCameras[nid]->isConncected()) {
            std::shared_ptr<kmod::MatTimeStamp>  mymat = nullptr;

            if (nid >= 0 && nid < _cmrSum) {
                mymat = kmod::g_ImgbufList[nid].pop();
                if (mymat != nullptr && !mymat->imgmat.empty()) {

                    cv::Mat srcImage = mymat->imgmat.clone();

                    // return srcImage;
                    cv::Mat dstImage;
                    if( camCalibs.size()>nid && camCalibs[nid]->useCalibrate) {
                        camCalibs[nid]->calibrateImage(srcImage,dstImage);
                        return dstImage;

                    }//if( camCalibs.size()>nid && camCalibs[nid]->useCalibrate)

                }//if (mymat != nullptr && !mymat->imgmat.empty())

            }	//	if (nid >= 0 && nid < _cmrSum)

        }//if ( nid >=0 && nid <_cmrSum  && kmod::g_pDevDatav->mCameras[nid]->isConncected())

        ///////////////////////////

        if( nid==-1) {

            cv::Mat  showmat,dst,src,mapmat, showmat2;

            showmat2 = cv::Mat::zeros( CameraParameter::BigimgSize, CV_8UC3);

            std::vector<cv::Mat>	imgvecs;

            for( int i=0; i<4; i++ ) {

                std::shared_ptr<kmod::MatTimeStamp>  mymat = kmod::g_ImgbufList[i].pop();

                if( mymat == nullptr) {
                    continue;
                }
                if( mymat->imgmat.empty()) {
                    continue;
                }
                if( camCalibs[i]->useCalibrate) {

                    camCalibs[i]->calibrateImage(mymat->imgmat,dst,mapmat);
                    imgvecs.push_back(mapmat.clone());

                    if(showmat.empty() ) {
                        showmat = cv::Mat(dst.rows, dst.cols * 4, dst.type());
                    }

                    cv::Rect roi_rect = cv::Rect( dst.cols*i, 0, dst.cols, dst.rows);
                    dst.copyTo(showmat(roi_rect));
                }
            }//	for( int i=0; i<4; i++ )


            /*
            if( camCalibs[nid]->useCalibrate){
            	//camCalibs[nid]->calibrateImage(mymat->imgmat,dst);
            	camCalibs[nid]->calibrateImage(mymat->imgmat,dst,mapmat);
            	mapmat.copyTo(showmat2, camCalibs[nid]->mask);
            	src = dst.clone();
            }else{
            	dst = mymat->imgmat.clone();
            	src = mymat->imgmat.clone();
            }
            // cv::pyrDown(dst,src,cv::Size(mymat->imgmat.cols, mymat->imgmat.rows));
            cv::Rect roi_rect = cv::Rect(0, 0, src.cols, src.rows);

            roi_rect = cv::Rect(src.cols*i, 0, src.cols, src.rows);

            if( i!=0){
            	roi_rect = cv::Rect(src.cols*i-src.cols/2, 0, src.cols, src.rows);
            	src.copyTo(shwomat(roi_rect));
            	src.release();
            }else{

            	cv::Rect roirect1 = cv::Rect(0, 0, src.cols/2, src.rows);
            	cv::Rect roirect2 = cv::Rect(src.cols/2, 0, src.cols/2, src.rows);
            	cv::Mat left = src(roirect1);
            	cv::Mat right = src(roirect2);
            	right.copyTo(shwomat(roirect1));
            	roirect2 = cv::Rect(3*src.cols+src.cols/2, 0, src.cols/2, src.rows);
            	left.copyTo(shwomat(roirect2));

            }


            */

            // if( i ==1 ){//left

            //     roi_rect = cv::Rect(src.cols*2, 0, src.cols, src.rows);
            // }
            // else if( i==2 ){//back

            //     roi_rect = cv::Rect(src.cols, 0, src.cols, src.rows);
            // }
            // else if( i==3 ){//left

            //     roi_rect = cv::Rect(0, 0, src.cols, src.rows);
            // }
            // else if( i==0 ){//front

            //     roi_rect = cv::Rect(src.cols*3, 0, src.cols, src.rows);
            // }




            CameraParameters::Fusion(showmat2,imgvecs);
            for( int i=0; i<4; i++ ) {
                //imgvecs
                imgvecs[i].copyTo(showmat2, camCalibs[i]->mask);
            }
            cv::rectangle(showmat2,CameraParameter::cvCentorRC,cv::Scalar(255,0,0),-1 );
            if( !showmat2.empty() ) {
                return showmat2;
            } else {
                _defaultmat = kmod::MyUtil::getDefaultMat();
                return _defaultmat;
            }
        }

        return cv::Mat();//QImage();
    }

private:
    //QTime xtime;
    //QElapsedTimer 						xtime;
    //std::shared_ptr<cv::VideoCapture>	cap = NULL;
    cv::Mat							_defaultmat;
    int									_cmrSum = 1;
    int									_nWidth = 1280;
    int									_nHeight = 720;
    std::vector<std::shared_ptr<CameraCalibrator>>  camCalibs;
};

struct MainWindow::Private {
    CGetOutImage out;
    QTimer timer;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _ncamearid = 0;

//------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------


    ui->mainToolBar->setIconSize(QSize(48, 48));
    //ui->mainToolBar->setIconSize(QSize(256, 256));
    ui->dockWidgetcalib->setWindowFlags(Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    ui->mainToolBar->setWindowIconText("Toolbar");
    setWindowIcon(QIcon(":/image/images/app.png"));
    // setWindowTitle(tr("Running"));//运行测试
    //alarmtable
    //alarmTableInit();
    //draw area
    ui->imgwidget->setMouseTracking(true);
    ui->scrollArea->setAlignment(Qt::AlignCenter); // 居中对齐
    QPalette pal(ui->scrollArea->palette());
    pal.setColor(QPalette::Background, Qt::darkGray);
    ui->scrollArea->setAutoFillBackground(true);
    ui->scrollArea->setPalette(pal);


    //camera
    //cmrpropertyinit();
    ui->imgwidget->setMouseTracking(true);
    int ncmr = kmod::limitV<int>(
                   kmod::KConfig::Instance().cfg()->get<int>("camerasum"), 5, 1);

    for (int i = 0; i < ncmr; i++)
        ui->camearcmb->addItem(QString::number( i + 1, 10));

    ui->camearcmb->blockSignals(false);

    connect(ui->camearcmb, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(OncamearcmbIndexChanged(QString)));
    //tool bar for camera channel switch
    for (int i = 0; i < ncmr; i++) {
        QAction *newact = new QAction("Camera:"+QString::number(i + 1));
        newact->setCheckable(true);
        newact->setShortcut(QKeySequence::fromString("CTRL+" + QString::number(i+1) ) );
        newact->setIcon(QIcon(QString(":/toolbar/images/%1.png").arg(i + 1)));
        _macts.append(newact);
    }

    ui->mainToolBar->addActions(_macts);
    ui->menu_Camera->insertActions(ui->actionZoomIn,_macts);
    ui->menu_Camera->insertSeparator(ui->actionZoomIn);
    _macts[0]->setChecked(true);

    // camera id toolbar action
    for (int i = 0; i < ncmr; i++) {
        connect(_macts[i], SIGNAL(triggered()), this, SLOT(OnSelectedCmr()));
    }
    //for btn
    connect(ui->btnIntriCalib, SIGNAL(clicked()), this, SLOT(OnbtnIntriCalibClicked()));
    connect(ui->btnExtriCalib, SIGNAL(clicked()), this, SLOT(OnbtnExtriCalibClicked()));
    connect(ui->tabShowWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabCurrentChanged(int)));


    tabifyDockWidget(ui->dockWidgetcalib, ui->dockWidgetcmr);
    tabifyDockWidget(ui->dockWidgetcmr, ui->dockWidgetalarm);

    //layout
    ui->dockWidgetcalib->setWindowFlags(Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint);
    _layoutSettings = new QSettings(QCoreApplication::applicationDirPath() + "/layout_config.ini",
                                    QSettings::IniFormat);
    _layoutSettings->setIniCodec(QTextCodec::codecForName("utf-8"));
    restoreGeometry(_layoutSettings->value("Layout/Geometry").toByteArray());
    restoreState(_layoutSettings->value("Layout/State").toByteArray());
    QList<QDockWidget* > dwList = this->findChildren<QDockWidget*>();
    foreach(QDockWidget * dw, dwList) {
        restoreDockWidget(dw);
    }
    //statusBar

    //_statusLabel = new QLabel(QStringLiteral("Running运行"));
    //_statusLabel = new QLabel(QString::fromLocal8Bit("Running运行"));
    _statusLabel = new QLabel("Running");//运行
    _statusLabel->setAlignment(Qt::AlignCenter);
    _statusLabel->setMinimumSize(_statusLabel->sizeHint());

    //ui->actionCapture->setEnabled(false);
    //QLabel* aixLabel;
    //aixLabel = new QLabel("\"CTRL + H\" for help");

    statusBar()->setStyleSheet(QString("QStatusBar::item{border: 0px}"));
    statusBar()->setSizeGripEnabled(true);
    statusBar()->addWidget(_statusLabel);
    //statusBar()->addWidget(aixLabel, 1);
    //show
    ui->tabShowWidget->setCurrentIndex(1);
    //ui->tabShowWidget->setTabEnabled(0, false);

    //  ui->gpyrend
    ui->openGLWidget->setRender(kmod::g_pRender);
//----------------------------------------------------------------------------
    pv = new Private();
    connect(&pv->timer, SIGNAL(timeout()), this, SLOT(doUpdate()));
    pv->timer.setInterval(100);
    pv->timer.start();
    // QTimer *timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, this, &SurroundViewDialog::animate);
    // timer->start(33);

//-----------------------------------------------------
    OnSelectedMode();
}

MainWindow::~MainWindow()
{
    delete pv;
    delete ui;
}
void MainWindow::onTabCurrentChanged(int index)
{
    _nTabIndex = index;
    //qDebug() << "Page change to: " << index;
}

void  MainWindow::OnSelectedMode()
{



    if ( 0 ==kmod::g_svcfg.nMode ) {
        ui->actionSingleMode->setChecked(true);

        ui->actionTop2D_Mode->setChecked(false);
        ui->actionTop3D_Mode->setChecked(false);
    }
    else if ( 1 ==kmod::g_svcfg.nMode ) {
        ui->actionSingleMode->setChecked(false);
        ui->actionTop2D_Mode->setChecked(true);
        ui->actionTop3D_Mode->setChecked(false);
    }
    else if ( 2 ==kmod::g_svcfg.nMode ) {
        ui->actionSingleMode->setChecked(false);
        ui->actionTop2D_Mode->setChecked(false);
        ui->actionTop3D_Mode->setChecked(true);
    }
    else {
        ui->actionSingleMode->setChecked(false);
        ui->actionTop2D_Mode->setChecked(false);
        ui->actionTop3D_Mode->setChecked(false);

    }

}
void MainWindow::OnSelectedCmr()
{
    QObject* obj = sender();
    QAction* pBtn = dynamic_cast<QAction*>(obj);
    int ncmr = _macts.size();

    for (int i = 0; i < ncmr; i++) {
        if (pBtn == _macts[i]) {
            _ncamearid = i;
            break;
        }
    }
    updateCmrSelectedid();
    //updatecmrproperty();
}
//switch camera combox
void MainWindow::OncamearcmbIndexChanged(const QString& context)
{
    int ncurindx = ui->camearcmb->currentIndex();
    _ncamearid = ncurindx;
    updateCmrSelectedid();
    // updatecmrproperty();
}
//update for switching camera
void MainWindow::updateCmrSelectedid()
{
    int ncmr = _macts.size();
    for (int i = 0; i < ncmr; i++) {
        if (i != _ncamearid) {
            _macts[i]->setChecked(false);
        }
        else {
            _macts[i]->setChecked(true);
        }
    }
    ui->camearcmb->setCurrentIndex(_ncamearid);
    ui->imgwidget->setCameraID(_ncamearid);
    ui->imgwidget->update();
}

//for zoominout image action
void MainWindow::on_actionZoomOut_triggered()
{
    if (_imgScale > 0.25) {
        _imgScale *= 0.8;
        updateImageShow();
    }
}
void MainWindow::on_actionZoomIn_triggered()
{
    if (_imgScale < 5.0) {
        _imgScale *= 1.25;
        updateImageShow();
    }
}

void MainWindow::on_actionsize100_triggered()
{
    _imgScale = 1.0;
    updateImageShow();
}

// update for zoominout
void MainWindow::updateImageShow()
{
    // visionConfig cfg = kmod::g_pDevDatav->mVisionCfgs[_ncamearid];
    ui->imgwidget->setFixedSize(_imgWidth * _imgScale, _imgHeight * _imgScale);
    ui->imgwidget->setSetScale(_imgScale);
    // cv::Rect myrc = cfg.getroi();
    // //QRect rc(cfg.roi.x, cfg.roi.y, cfg.roi.width, cfg.roi.height);
    // QRect rc(myrc.x, myrc.y, myrc.width, myrc.height);
    // ui->imgwidget->setDetectArea(rc);
    ui->imgwidget->update();

}

void MainWindow::on_actionCapture_triggered()
{
    //if (1 == kmod::g_pDevDatav->getWorkMode())
    {
        //
        QString pdfdate = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH_mm_ss");
        QString pdfPath = QCoreApplication::applicationDirPath() + "/" +
                          pdfdate +  "_ch" +
                          QString::number(_ncamearid+1)  +
                          ".png";


        ui->imgwidget->saveCurrImage(pdfPath);
        //qDebug() << pdfPath;
    }
}

//update data
void MainWindow::doUpdate()
{

    int nch =_ncamearid;
    //if( kmod::g_svcfg.nMode !=0 ) nch = -1;

    cv::Mat imMat = pv->out.getNextFrame(nch);


    if( 0 == _nTabIndex) {

        if( !imMat.empty() ) {
            QImage image = kmod::MyUtil::MatToQImage(imMat);
            int w = image.width();
            int h = image.height();
            _imgWidth = w;
            _imgHeight = h;
            ui->imgwidget->setFixedSize(w * _imgScale, h * _imgScale);
            ui->imgwidget->setImage(image);
            ui->scrollArea->setBackgroundRole(QPalette::Dark);

        }


    } else if(1 == _nTabIndex) {
        if( !imMat.empty() ) {
            _imgWidth = imMat.cols;
            _imgHeight = imMat.rows;
            ui->openGLWidget->setBuffer(imMat);
        }
        ui->openGLWidget->update();
    }



    //update dock status
    if (ui->dockWidgetcmr->isHidden()) {

        ui->actioncamera->setChecked(false);
    }
    else {
        ui->actioncamera->setChecked(true);
    }
    if (ui->dockWidgetcalib->isHidden()) {

        ui->actionCalib_Setting->setChecked(false);
    }
    else {
        ui->actionCalib_Setting->setChecked(true);
    }
    if (ui->dockWidgetalarm->isHidden()) {

        ui->actionAlarm_Window->setChecked(false);
    }
    else {
        ui->actionAlarm_Window->setChecked(true);
    }
    if (ui->dockWidgetoutput->isHidden()) {

        ui->actionOutput_Windows->setChecked(false);
    }
    else {
        ui->actionOutput_Windows->setChecked(true);
    }
    if (ui->dockWidgetSettings->isHidden()) {

        ui->actionSettings_Window->setChecked(false);
    }
    else {
        ui->actionSettings_Window->setChecked(true);
    }



    //information for output windows
    if (kmod::g_infos->isNewer()) {
        std::list<std::string> strs = kmod::g_infos->getinfos();
        ui->outputlist->clear();

        for (const std::string &str : strs) {
            //ui->outputlist->addItem(QString::fromStdString(var));
            //ui->outputlist->insertItem(0, QString::fromStdString(var));
            ui->outputlist->insertItem(0, QString::fromLocal8Bit(str.c_str() ) );
        }

    }


}


//for configration or detecting
void MainWindow::on_actionSingleMode_triggered()
{

    kmod::g_svcfg.nMode=0;
    OnSelectedMode();

}
void MainWindow::on_actionTop2D_Mode_triggered()
{
    kmod::g_svcfg.nMode=1;
    OnSelectedMode();

}
void MainWindow::on_actionTop3D_Mode_triggered()
{
    kmod::g_svcfg.nMode=2;
    OnSelectedMode();
}
void MainWindow::on_actioncamera_triggered()
{
    if (ui->actioncamera->isChecked()) {
        ui->dockWidgetcmr->show();
    } else {
        ui->dockWidgetcmr->hide();
    }
}
void MainWindow::on_actionCalib_Setting_triggered()
{
    if (ui->actionCalib_Setting->isChecked()) {
        ui->dockWidgetcalib->show();
    }	else {
        ui->dockWidgetcalib->hide();
    }

}
void MainWindow::on_actionAlarm_Window_triggered()
{
    if (ui->actionAlarm_Window->isChecked()) {
        ui->dockWidgetalarm->show();
    } else {
        ui->dockWidgetalarm->hide();
    }
}
void MainWindow::on_actionSettings_Window_triggered()
{
    if (ui->actionSettings_Window->isChecked()) {
        ui->dockWidgetSettings->show();
    }
    else {
        ui->dockWidgetSettings->hide();
    }

}
void MainWindow::on_actionOutput_Windows_triggered()
{
    if (ui->actionOutput_Windows->isChecked()) {
        ui->dockWidgetoutput->show();
    } else {
        ui->dockWidgetoutput->hide();
    }

}
void MainWindow::on_actionToolbar_triggered()
{
    if (ui->actionToolbar->isChecked()) {
        ui->mainToolBar->show();
    }
    else {
        ui->mainToolBar->hide();
    }

}
void MainWindow::on_action_ABout_triggered()
{
    aboutdlg* dlg = new aboutdlg();
    dlg->show();
}




// alarm window init
void MainWindow::alarmTableInit()
{
    QStringList header;
    header << tr("time") << tr("type") << tr("channel") << tr("file");//<< tr("details");
    ui->alarmtable->setColumnCount(header.size());
    ui->alarmtable->setHorizontalHeaderLabels(header);
    //QHeaderView::ResizeToContents	//Stretch
    //ui->alarmtable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    //ui->alarmtable->setRowCount(5);
    ui->alarmtable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->alarmtable, SIGNAL(cellDoubleClicked(int, int)),
            this, SLOT(OncellDoubleClicked(int, int)));

    //int n = ui->alarmtable->rowCount();
    // size_t ndd = kmod::g_OutAlarm.size();
    // if (ndd > 0) {
    // 	for (int i = 0; i < ndd; i++) {
    // 		QString  stritem;
    // 		int rowid = n + i;
    // 		std::string strtime = kmod::g_OutAlarm.dat()[i].strtimestamp;
    // 		ui->alarmtable->insertRow(rowid);
    // 		ui->alarmtable->setItem(rowid, 0, new QTableWidgetItem(
    // 			QString::fromStdString(strtime)));

    // 		ui->alarmtable->setItem(rowid, 1, new QTableWidgetItem(
    // 			QString::number(kmod::g_OutAlarm.dat()[i].type )));

    // 		ui->alarmtable->setItem(rowid, 2, new QTableWidgetItem(
    // 			QString::number(kmod::g_OutAlarm.dat()[i].channel)));
    // 		ui->alarmtable->setItem(rowid, 3, new QTableWidgetItem(
    // 			QString::fromStdString(kmod::g_OutAlarm.dat()[i].strimgfile)));

    // 	}
    // }
}

void MainWindow::updateAlarmTable()
{
    // int n = ui->alarmtable->rowCount();
    // size_t ndd = kmod::g_OutAlarm.size();
    // int rowid = 0;
    // if (n < ndd) {
    // 	//
    // 	int ncount = ndd - n;
    // 	//qDebug() << ncount;
    // 	QColor linecolor = QColor(0, 0, 0);
    // 	// for (int j = ncount - 1; j >= 0; j--) {

    // 	// 	ui->alarmtable->insertRow(rowid);

    // 	// 	//time
    // 	// 	std::string strtime = kmod::g_OutAlarm.dat()[j].strtimestamp;

    // 	// 	ui->alarmtable->setItem(rowid, 0, new QTableWidgetItem(
    // 	// 		QString::fromStdString(strtime)));

    // 	// 	//type
    // 	// 	int ntype = kmod::g_OutAlarm.dat()[j].type;
    // 	// 	ui->alarmtable->setItem(rowid, 1, new QTableWidgetItem(
    // 	// 		QString::number(ntype)));


    // 	// 	//ch
    // 	// 	int ch = kmod::g_OutAlarm.dat()[j].channel;
    // 	// 	ui->alarmtable->setItem(rowid, 2, new QTableWidgetItem(
    // 	// 		QString::number(ch)));

    // 	// 	ui->alarmtable->setItem(rowid, 3, new QTableWidgetItem(
    // 	// 		QString::fromStdString(kmod::g_OutAlarm.dat()[j].strimgfile)));


    // 	// }
    // }

}

void MainWindow::OnbtnIntriCalibClicked()
{
    //CameraListWidget       cameraList(CameraListWidget::intrinsic, this);
    _cameraList = new CameraListWidget(CameraListWidget::intrinsic, this);
    _cameraList->setAttribute(Qt::WA_DeleteOnClose);
    _cameraList->setWindowTitle("Camera List-Intrinsic");
    _cameraList->exec();
}
void MainWindow::OnbtnExtriCalibClicked()
{

    _cameraList = new CameraListWidget(CameraListWidget::extrinsic,this);
    _cameraList->setAttribute(Qt::WA_DeleteOnClose);
    _cameraList->setWindowTitle("Extrinsic List-Intrinsic");
    _cameraList->exec();
}




void MainWindow::on_action_FullScreen_triggered()
{
    if (ui->action_FullScreen->isChecked()) {
        this->setWindowFlags(Qt::FramelessWindowHint);
        this->showFullScreen();
    }
    else {
        this->showNormal();
    }

}
void MainWindow::on_action_Exit_triggered()
{
    close();
}
void MainWindow::closeEvent(QCloseEvent* event)
{
    switch (QMessageBox::information(this, tr("Detector"),
                                     tr("Do you really want to Exit?"),
                                     tr("Yes"), tr("No"),0, 1)) {
    case 0:
        event->accept();
        _layoutSettings->setValue("Layout/Geometry", saveGeometry());
        _layoutSettings->setValue("Layout/State", saveState());
        _layoutSettings->sync();
        break;
    case 1:
    default:
        event->ignore();
        break;
    }

}

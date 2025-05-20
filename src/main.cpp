#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <QApplication>
#include <QLocale>
#include <QFileInfo>
#include <QTranslator>
#include <QSplashScreen>
#include <Util/logger.h>
#include <Util/CMD.h>
#include <Util/util.h>
#include <Util/RingBuffer.h>
#include <Thread/threadgroup.h>
#include <Util/TimeTicker.h>
#include <Poller/Timer.h>
#include "rtspplayer.h"
#include "KConfig.h"
#include "KMsg.h"
#include "MyUtil.h"
#include "mainwindow.h"
#include "cameraparameter.h"

#include "Datadefine.h"


using namespace toolkit;


namespace kmod {
std::shared_ptr<DataPrivate>  g_pDevDatav(new DataPrivate());
std::shared_ptr<InfosPrivate> g_infos(new InfosPrivate());
std::vector<std::shared_ptr<kmod::CameraParameter>> g_camerasParam;
std::shared_ptr<SVRender> g_pRender;
std::shared_ptr<Camera>   g_pcamera;
SVAppConfig               g_svcfg;

void addCar(std::shared_ptr<SVRender>& view_, const SVAppConfig& svcfg)
{
    glm::mat4 transform_car(1.f);
#ifdef HEMISPHERE
    transform_car = glm::translate(transform_car, glm::vec3(0.f, 0.09f, 0.f));
#else
    transform_car = glm::translate(transform_car, glm::vec3(0.f, 1.01f, 0.f));
#endif

    transform_car = glm::rotate(transform_car, glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
    transform_car = glm::scale(transform_car, glm::vec3(0.002f));

    bool is_Add = view_->addModel(svcfg.car_model, svcfg.car_vert_shader,
                                  svcfg.car_frag_shader, transform_car);
    if (!is_Add)
        std::cerr << "Error can't add model\n";
}

void addBowlConfig(ConfigBowl& cbowl)
{
    /* Bowl parameter */
    glm::mat4 transform_bowl(1.f);
    cbowl.transformation = transform_bowl;
    cbowl.disk_radius = 0.4f;
    cbowl.parab_radius = 0.56f;
    cbowl.hole_radius = 0.08f;
    cbowl.a = 0.4f;
    cbowl.b = 0.4f;
    cbowl.c = 0.2f;
    cbowl.vertices_num  = 150.f;
    cbowl.y_start = 1.0f;
}


void resetCameraState()
{
    kmod::g_pcamera = std::make_shared<kmod::Camera>(glm::vec3(0.0, 1.7, 1.0), glm::vec3(0.0, 1.0, 0.0));
}


}


int main(int argc, char *argv[]) try
{








    // QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
//application---------------------------------------
    QApplication app(argc, argv);
    QTranslator translator;





    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "cameraFusion_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            break;
        }
    }



    QPixmap pixmap(":/image/images/loader.jpg");
    QSplashScreen splash(pixmap);
    splash.show();
    splash.update();
    app.processEvents();

    Logger::Instance().add(std::make_shared<ConsoleChannel>());
    Logger::Instance().add(std::make_shared<FileChannel>());
    Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());
    Logger::Instance().setLevel(LogLevel::LInfo);
    std::string exepath = kmod::MyUtil::getExePath();
    std::string mycfgpath = exepath + "/../cameraFusion.config";
    std::string cmrfgpath = exepath + "/../data/cameras.xml";
    std::string cmrcfgpath = exepath + "/../data";
    kmod::g_infos->append(exepath);
    kmod::g_infos->append(mycfgpath);
    InfoL<<exepath;
    InfoL<<mycfgpath;
    InfoL<<cmrfgpath;
    InfoL<<cmrcfgpath;

    app.processEvents();

    splash.showMessage(QStringLiteral("load file"), Qt::AlignHCenter|Qt::AlignBottom, Qt::white);  //设置消息
    //用户可以通过用鼠标单击来隐藏启动画面。由于启动画面通常在事件循环开始运行之前显示，因此有必要定期调用 QCoreApplication::processEvents() 以接收鼠标点击。
    QCoreApplication::processEvents();
    kmod::KConfig::Instance().setcfgfile(mycfgpath);
//---------------------------------------------------
    //std::vector<std::shared_ptr<rtspPlayer>> mRstps;
    int ncmr = kmod::limitV<int>( kmod::KConfig::Instance().cfg()->get<int>("camerasum"), 8, 1 );
    InfoL<<"camera:"<<ncmr;
    for( int i=0; i<ncmr; i++) {
        std::string stritem = "camera_string_" + std::to_string(i);
        string strcamera = kmod::KConfig::Instance().cfg()->get<std::string>(stritem);
        InfoL<< i <<" camera string :"<<strcamera;
        kmod::g_pDevDatav->mCameras.emplace_back( std::make_shared<rtspPlayer>(strcamera));
        app.processEvents();
    }

    std::vector<std::string> camNames;
    camNames.emplace_back("front");
    camNames.emplace_back("left");
    camNames.emplace_back("back");
    camNames.emplace_back("right");

    int nid=0;
    for(auto &name: camNames) {
        std::shared_ptr<kmod::CameraParameter> p = std::make_shared<kmod::CameraParameter>();
        p->name = name;
        p->devId = nid++;
        kmod::g_camerasParam.emplace_back(p);
        app.processEvents();
    }


    QFileInfo checkFile(QString::fromStdString(cmrfgpath));
    if(checkFile.exists() && checkFile.isFile()) {
        InfoL<< "loading " << cmrfgpath << "!";
        kmod::CameraParameters::loadCamerasParam(cmrfgpath,kmod::g_camerasParam);
    } else {
        InfoL<< cmrfgpath << ": not found";
        kmod::CameraParameters::saveCamerasParam(cmrfgpath,kmod::g_camerasParam);
    }
    kmod::CameraParameters::loadCmrParam(cmrcfgpath,kmod::g_camerasParam);


    // kmod::addBowlConfig(kmod::g_svcfg.cbowl);
    // std::vector<float> data;
    // std::vector<uint> idxs;
    // kmod::Bowl bowl(kmod::g_svcfg.cbowl);
    // bowl.generate_mesh_uv_hole(kmod::g_svcfg.cbowl.vertices_num,kmod::g_svcfg.cbowl.hole_radius, data, idxs);
    // return 0;

//---------------------------------------------------
    int i = 0;
    for_each(kmod::g_pDevDatav->mCameras.begin(),kmod::g_pDevDatav->mCameras.end(),[&](std::shared_ptr<rtspPlayer> rstp) {

        splash.showMessage(QStringLiteral("connecting to:") + QString::fromStdString(rstp->getpath()), Qt::AlignHCenter|Qt::AlignBottom, Qt::white);   //设置消息
        app.processEvents();
        rstp->connect();
        if(rstp->isConncected()) {
            InfoL<<"Connecting to "<<rstp->getpath()<<". W="<<rstp->getWidth()<<" H="<<rstp->getHeight()<<" OK!";

            kmod::g_infos->append(rstp->getpath());
            std::thread(&rtspPlayer::decode, rstp, i ).detach();

        } else {
            ErrorL<<"Connecting to "<<rstp->getpath()<<"Failed!";
        }
        i++;
        app.processEvents();
    });
//application---------------------------------------
    MainWindow win;
    QSize size = win.size();
    kmod::g_pcamera = std::make_shared<kmod::Camera>(glm::vec3(0.0, 1.7, 1.0), glm::vec3(0.0, 1.0, 0.0));
    kmod::g_pRender =  std::make_shared<kmod::SVRender>(size.width(),size.height());
    win.show();
    splash.finish(&win);
    app.exec();
//------------------------------------------------
    if(kmod::CameraParameter::changed) {
        kmod::CameraParameters::saveCamerasParam(cmrfgpath,kmod::g_camerasParam);
    }
//-------------------------------------------------
    for (std::shared_ptr<rtspPlayer> rstp : kmod::g_pDevDatav->mCameras) {
        rstp->close();
    }
    //wait
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    return 0;
}
catch (const std::exception& e) {
    ErrorL << "Fatal Error:"<< e.what();;
    return EXIT_FAILURE;
}


















// mRstps.emplace_back( std::make_shared<rtspPlayer>("rtsp://admin:12345@192.168.10.161:8554/0"));
// mRstps.emplace_back( std::make_shared<rtspPlayer>("rtsp://admin:12345@192.168.10.162:8554/0"));
// mRstps.emplace_back( std::make_shared<rtspPlayer>("rtsp://admin:12345@192.168.10.163:8554/0"));
// mRstps.emplace_back( std::make_shared<rtspPlayer>("rtsp://admin:12345@192.168.10.164:8554/0"));

//thread_id1[i].detach();
//thread_id1[i] = std::thread(&rtspPlayer::decode, rstp, i );
//thread_id1[i].detach();

// InfoL<<"close";
//imgsproc.exitloop();
//  thread_id2[i] = std::thread(&ImageProc::process, players[i],rstp, i );
// thread_id2[i].detach();

// ImagesProc  imgsproc;
// std::thread  t2{ &ImagesProc::process2, &imgsproc,mRstps.size()};
// t2.detach();
// class ImageProc
// {

// public:
//     ImageProc(int channel){
//         _ch = channel;
//         std::cout<<"ImagesProc " <<"channel="<<_ch<<std::endl;
//     }
//     ~ImageProc(){}
//     void process(std::shared_ptr<rtspPlayer> decoder,int ch);
//  private:
//     int _ch=0;
// };

// void ImageProc::process(std::shared_ptr<rtspPlayer> xdecoder,int nid)
// {
//     std::shared_ptr<rtspPlayer> decoder = xdecoder;
//     cv::namedWindow( "src"+std::to_string(nid), 0 );
//     int icount=0;
//     while(decoder->isConncected()){
//         std::shared_ptr<kmod::MatTimeStamp>  mymat = kmod::g_ImgbufList[nid].pop();

//         if( mymat == nullptr){
//            std::this_thread::sleep_for(std::chrono::milliseconds(20));
//            std::cout<<"nullptr:"<<nid<<std::endl;
//            continue;

//         }

//         if( mymat->imgmat.empty()){
//            std::this_thread::sleep_for(std::chrono::milliseconds(20));
//             std::cout<<"empty:"<<nid<<std::endl;
//            continue;
//         }

//        cv::imshow("src"+std::to_string(nid), mymat->imgmat);

//     }
// }

// class ImagesProc
// {

// public:
//     ImagesProc(){

//     }
//     ~ImagesProc(){}
//     void process2(int nsum);
//     void exitloop(){ _bloop = false;}
// private:
//     bool _bloop = true;

// };
// void ImagesProc::process2(int nsum)
// {
//     cv::Mat shwomat;
//     int ich = 0;
//     cv::namedWindow( "src", 0 );
//     //Mat result(sz.height*2+1, sz.width * 2 + 1, image_1.type());
//     _bloop = true;
//     while( _bloop ) {
//         ich = 0;

//         for( int i=0; i<nsum; i++ ) {


//             std::shared_ptr<kmod::MatTimeStamp>  mymat = kmod::g_ImgbufList[i].pop();
//             if( mymat == nullptr){
//                 continue;

//             }
//             if( mymat->imgmat.empty()){
//                 //std::this_thread::sleep_for(std::chrono::milliseconds(2));
//                 //std::cout<<"empty:"<<nid<<std::endl;
//                 continue;
//             }
//             if(shwomat.empty() ) {
//                 shwomat = cv::Mat(mymat->imgmat.rows + 1, mymat->imgmat.cols + 1, mymat->imgmat.type());
//             }
//             cv::Mat src;
//             cv::pyrDown(mymat->imgmat,src,cv::Size(mymat->imgmat.cols/2, mymat->imgmat.rows /2));

//             cv::Rect roi_rect = cv::Rect(0, 0, src.cols, src.rows);

//             if( i ==1 ){

//                 roi_rect = cv::Rect(src.cols, 0, src.cols, src.rows);
//             }
//             else if( i==2 ){

//                 roi_rect = cv::Rect(0, src.rows, src.cols, src.rows);
//             }
//             else if( i==3 ){

//                 roi_rect = cv::Rect(src.cols, src.rows, src.cols, src.rows);
//             }
//             else if( i==0 ){

//                 roi_rect = cv::Rect(0, 0, src.cols, src.rows);
//             }

//             src.copyTo(shwomat(roi_rect));
//             src.release();

//         }

//         if( !shwomat.empty())   cv::imshow("src", shwomat);
//         std::this_thread::sleep_for(std::chrono::milliseconds(20));
//     }
//     //std::cout<<"process2 exit:"<<std::endl;



// }

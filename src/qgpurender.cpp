#include "qgpurender.h"

#include <QElapsedTimer>
#include <QDebug>
#include <QEvent>
#include <QGesture>
// #include <QtMath>
// #include <QFile>

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3ext.h>
#include <Util/logger.h>
#include "visualObj.hpp"
#include "SVRender.hpp"

#include "Datadefine.h"

// #include <GL/glu.h>
// #include <GL/glut.h>
//#include "graphics.hpp"


#define EGL_API_FB
#include <EGL/egl.h>
#ifndef GL_VIV_direct_texture
#define GL_VIV_YV12                     0x8FC0
#define GL_VIV_NV12                     0x8FC1
#define GL_VIV_YUY2                     0x8FC2
#define GL_VIV_UYVY                     0x8FC3
#define GL_VIV_NV21                     0x8FC4
#endif
/* GL_VIV_clamp_to_border */
#ifndef GL_VIV_clamp_to_border
#define GL_VIV_clamp_to_border 1
#define GL_CLAMP_TO_BORDER_VIV         0x812D
#endif

extern "C"
{
#include <pthread.h>
}


// static const GLint eglTexture[4] = {
//     GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2, GL_TEXTURE3
// };


using namespace toolkit;

QGpuRender::QGpuRender(QWidget *parent) :
    QOpenGLWidget(parent)
{
    //setAttribute(Qt::AA_UseOpenGLES,true);
    //QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAttribute(Qt::WA_AcceptTouchEvents);
    setFocusPolicy(Qt::StrongFocus);
    _width = 1280;
    _height = 720;
    aspect_ratio = 1.0;
    isInit =false;
    bMouseDown = false;
    demo_key_press = false;
    topview_key_press = false;
    useDemoSurroundView= false;
    useDemoTopView= false;
}

QGpuRender::~QGpuRender()
{
    // makeCurrent();
    _prectobj->clearBuffers();
    doneCurrent();
}
void QGpuRender::setBuffer(const cv::Mat &img)
{
    _prectobj->Prepare(img);
    _frame = img.clone();
}
void QGpuRender::initializeGL()
{
    initializeOpenGLFunctions();

    _prectobj = std::make_shared<kmod::TestVis>();
    // _prectobj->Init();

    if (!kmod::g_pRender->getInit()) {

        kmod::addBowlConfig(kmod::g_svcfg.cbowl);
        if( !kmod::g_pRender->init(kmod::g_svcfg.cbowl, kmod::g_svcfg.surroundshadervert, kmod::g_svcfg.surroundshaderfrag,
                                   kmod::g_svcfg.screenshadervert, kmod::g_svcfg.screenshaderfrag,
                                   kmod::g_svcfg.blackrectshadervert, kmod::g_svcfg.blackrectshaderfrag,
                                   kmod::g_svcfg.bgshadervert, kmod::g_svcfg.bgshaderfrag)) {
            qDebug()<<"render init failed!";
        }
        kmod::addCar( kmod::g_pRender, kmod::g_svcfg);
    }

    std::cout<<"openglinit";
    glShadeModel(GL_SMOOTH);
    //glEnable(GL_CULL_FACE);
//glShadeModel(GL_FLAT);
// float light_ambient[] = { 1, 1, 1, 1 };
// //float light_diffuse[] = { -1, -1, -1, 1 };
// float light_diffuse[] = { 1, 1, 1, 1 };
// float light_specular[] = { 1, 1, 1, 1 };
// float light_position[] = { 0, 0, 100, 0 };
// glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
// glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
// glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
// glLightfv(GL_LIGHT0, GL_POSITION, light_position);
// glEnable(GL_LIGHT0);
// glDisable(GL_LIGHTING); // by default, disable lighting

    glEnable(GL_NORMALIZE); // calls to glNormal will result in normalized normal vectors
    //glClearColor(0.0, 0.0, 0.0, 1.0); // set background color (red,gree,blue,alpha): black
    glClearDepth(1.0); // set maximum depth
    isInit = true;
}

void QGpuRender::paintGL()
{
    if( !isInit ) return;

    if( !_prectobj->IsInit()) {
        _prectobj->Init();
    }
    //   glEnable(GL_DEPTH_TEST);
    //this->glViewport(0,0,w,h);
    //glBindFramebuffer(GL_FRAMEBUFFER, OGLquadrender.framebuffer); // bind scene framebuffer
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //drawbackground
    if( kmod::g_svcfg.nMode >1 ) {
        if (kmod::g_pRender->getInit()) {
            if( !_frame.empty()) {
                kmod::g_pRender->render( kmod::g_pcamera, _frame);
            }
        }
        if (useDemoSurroundView)
            demoSVMode();
        if (useDemoTopView)
            demoTVMode();
        // _prectobj->PaintGLOpaque();
    } else {
        _prectobj->PaintGLOpaque();
    }

    glFlush();


    //     glDisable(GL_BLEND); // disable transparency effects
    //     glDepthMask(false);
    //     glDepthMask(false);
//     glEnable(GL_BLEND); // enable transparent effects
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//     //glEnable(GL_BLEND);
//     //glBlendFunc(GL_ONE, GL_ONE);
//     //glBlendColor(0.0f, 0.0f, 0.0f, 0.0f);
// //  glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
// //  glBlendFunc(GL_DST_COLOR, GL_SRC_ALPHA); //SRC-factor : existing pixels, DST-factor: about to draw
//     glDepthMask(true);



}
void QGpuRender::demoSVMode()
{
    constexpr auto const_speed = 0.03;
    kmod::g_pcamera->processKeyboard(kmod::Camera_Movement::LEFT, const_speed);
    constexpr auto xoffset = 15.0;
    kmod::g_pcamera->processMouseMovement(xoffset, 0);
}

void QGpuRender::demoTVMode()
{
    constexpr auto angle = -90.0;
    constexpr auto rot_angle_x = 7.5;
    kmod::g_pcamera->processMouseMovement(0, angle);
    kmod::g_pcamera->processMouseMovement(rot_angle_x, 0);
}



void QGpuRender::setSVMode(const bool demo)
{
    useDemoSurroundView = demo;
    kmod::resetCameraState();
    kmod::g_pcamera->setCamPos(glm::vec3(0, 2.5, 2.75));
    kmod::g_pcamera->processMouseMovement(0, -315);
}

bool QGpuRender::getSVMode() const
{
    return useDemoSurroundView;
}

void QGpuRender::setTVMode(const bool topview)
{
    useDemoTopView = topview;
    kmod::resetCameraState();
    kmod::g_pcamera->setCamPos(glm::vec3(0, 4.15, 0.0));
}

bool QGpuRender::getTVMode() const
{
    return useDemoTopView;
}

void QGpuRender::resizeGL(int w, int h)
{
    _width = w;
    _height = h;
    aspect_ratio =  float(w) / float(h ? h : 1);
}
void QGpuRender::mousePressEvent(QMouseEvent *event)
{
    event->accept();
    last_mouse_x = event->x();
    last_mouse_y = event->y();
    bMouseDown = true;

}
void QGpuRender::mouseReleaseEvent(QMouseEvent *event)
{
    event->accept();
    bMouseDown = false;
}

void QGpuRender::mouseMoveEvent(QMouseEvent *event)
{
    event->accept();
    if( bMouseDown) {
        int dx = event->x() - last_mouse_x;
        int dy = event->y() - last_mouse_y;
        kmod::g_pcamera->processMouseMovement(dx, dy);
        last_mouse_x = event->x();
        last_mouse_y = event->y();

    }
}
void QGpuRender::wheelEvent(QWheelEvent*event)
{
    QPoint p = event->angleDelta();
    kmod::g_pcamera->processMouseScroll((float)p.y());
}
void  QGpuRender::KeyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_T:
        topview_key_press = false;
        //setSVMode(!getSVMode());
        break;
    case Qt::Key_V:
        demo_key_press = false;
        //setTVMode(!getTVMode());
        break;
    }
}
void QGpuRender::keyPressEvent(QKeyEvent *event)
{
    constexpr auto const_speed = 0.5f;

    switch (event->key()) {
    case Qt::Key_W:
        kmod::g_pcamera->processKeyboard( kmod::Camera_Movement::FORWARD, const_speed);
        break;
    case Qt::Key_S:
        kmod::g_pcamera->processKeyboard( kmod::Camera_Movement::BACKWARD, const_speed);
        break;
    case Qt::Key_A:
        kmod::g_pcamera->processKeyboard( kmod::Camera_Movement::LEFT, const_speed);
        break;
    case Qt::Key_D:
        kmod::g_pcamera->processKeyboard( kmod::Camera_Movement::RIGHT, const_speed);
        break;
    case Qt::Key_T:
        if(  !demo_key_press) {
            setTVMode(false);
            setSVMode(!getSVMode());
            demo_key_press = true;
        }
        break;
    case Qt::Key_V:
        if(  !topview_key_press) {
            setSVMode(false);
            setTVMode(!getTVMode());
            topview_key_press = true;
        }
        break;

    case Qt::Key_Space:

        setSVMode(false);
        setTVMode(false);
        kmod::resetCameraState();
        break;

    }


}

bool QGpuRender::event(QEvent *e)
{
    if(e->type() == QEvent::TouchBegin) {
        return true;
    }
    else if(e->type() == QEvent::TouchEnd) {
        return true;
    }
    else if(e->type() == QEvent::TouchUpdate) {
//        qInfo() << "touch update";
        // QTouchEvent *touchEvent = static_cast<QTouchEvent*>(e);
        // auto points = touchEvent->touchPoints();
        // auto sizePoints = points.size();
        return true;
    }

    return QOpenGLWidget::event(e);
}

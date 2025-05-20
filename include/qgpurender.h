#ifndef QGPU_RENDER_H
#define QGPU_RENDER_H

// #include <QOpenGLTexture>
// #include <QOpenGLShaderProgram>
// #include <QOpenGLBuffer>
// #include <QOpenGLVertexArrayObject>
// #include "qcameraop.h"
#include <vector>
#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <opencv2/opencv.hpp>
#include "Virtcam.hpp"

using int32 = int32_t;
namespace kmod {
class SVRender;
class VisualObj;
}

QT_BEGIN_NAMESPACE
class QGestureEvent;
QT_END_NAMESPACE

class QGpuRender : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
    Q_OBJECT
public:
    explicit QGpuRender(QWidget *parent = 0);
    ~QGpuRender();
    void setRender(std::shared_ptr<kmod::SVRender>  view_) {
        _disp_view = view_;
    }
    void setBuffer(const cv::Mat &img);
    void setSVMode(const bool demo);
    bool getSVMode() const;
    void setTVMode(const bool topview);
    bool getTVMode() const;
    void demoSVMode();
    void demoTVMode();
public slots:
protected:
    virtual void initializeGL() Q_DECL_OVERRIDE;
    virtual void paintGL() Q_DECL_OVERRIDE;
    virtual void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    virtual bool event(QEvent *e) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void KeyReleaseEvent(QKeyEvent *event);
    virtual void wheelEvent(QWheelEvent*event) Q_DECL_OVERRIDE;
private:
    int last_mouse_x, last_mouse_y;
    int32 _width, _height;
    float aspect_ratio;
    bool isInit;
    std::shared_ptr<kmod::SVRender>  _disp_view;
    std::shared_ptr<kmod::VisualObj> _prectobj;
    cv::Mat _frame;
    bool bMouseDown = true;
    bool demo_key_press = false;
    bool topview_key_press = false;
    bool useDemoSurroundView, useDemoTopView;
};

#endif // GPU_RENDER_H

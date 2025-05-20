#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QMap>
#include <QSettings>
#include <QLabel>
#include <QVector>
#include "cameralistwidget.h"
#include "cameraparameter.h"


class QtProperty;
class QtVariantProperty;


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    struct Private;
    Private* pv;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void cmrpropertyinit();
    void alarmTableInit();

private slots:

    void doUpdate();
    //-
    void on_actioncamera_triggered();
    void on_actionCalib_Setting_triggered();
    void on_actionAlarm_Window_triggered();
    void on_actionOutput_Windows_triggered();
    void on_actionSettings_Window_triggered();
    void on_actionToolbar_triggered();
    void on_action_FullScreen_triggered();

    void on_actionCapture_triggered();
    //-
    void on_action_ABout_triggered();
    void on_action_Exit_triggered();
    //-
    void on_actionZoomOut_triggered();
    void on_actionZoomIn_triggered();
    void on_actionsize100_triggered();


    //--
    void on_actionSingleMode_triggered();
    void on_actionTop2D_Mode_triggered();
    void on_actionTop3D_Mode_triggered();

    void closeEvent(QCloseEvent* event);
    void OncamearcmbIndexChanged(const QString& context);
    void OnbtnIntriCalibClicked();
    void OnbtnExtriCalibClicked();
    void OnSelectedCmr();
    void OnSelectedMode();


    void onTabCurrentChanged(int );
private:
    void updateAlarmTable();
    void updateCmrSelectedid();
    void updateImageShow();

private:

    Ui::MainWindow *         ui;
    QList<QAction*>         _macts;
    int                     _ncamearid = 0;
    float                   _imgScale=1.0;
    int                     _imgWidth=2048;
    int                     _imgHeight=1024;
    QSettings*              _layoutSettings;
    QLabel*                 _statusLabel;

    bool                    _intrinsicAllReady = false;
    bool                    _extrinsicAllReady = false;
    CameraListWidget  *     _cameraList = 0;

    int                     _nTabIndex=0;

};
#endif // MAINWINDOW_H

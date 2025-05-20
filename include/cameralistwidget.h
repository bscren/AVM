#ifndef CAMERALIST_H
#define CAMERALIST_H

#include <QDialog>
#include <QLabel>

#include "calibratewidget.h"
#include "cameraparameter.h"
#include <vector>

namespace Ui {
class CameraListWidget;
}

class CameraListWidget : public QDialog
{
    Q_OBJECT
public:
    enum Attribute { intrinsic = 0,  extrinsic, };    
    explicit CameraListWidget(Attribute attr, QWidget *parent = 0);
    ~CameraListWidget();
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
private slots:
private:
    void setCalibrateStatus(QLabel *label, const std::shared_ptr<kmod::CameraParameter> &p, bool ready);
    Ui::CameraListWidget *ui;
  //  std::vector<std::shared_ptr<kmod::CameraParameter>> pvec;
};

#endif // CAMERALIST_H

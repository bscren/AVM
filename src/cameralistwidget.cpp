#include "cameralistwidget.h"
#include "extrinsicdialog.h"
#include "aspectsinglelayout.h"
#include "ui_cameralistwidget.h"

#include <functional>

#include <QDebug>
#include <QImage>
#include <QScrollBar>
#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include "Datadefine.h"
#include "KMsg.h"
#include "MyUtil.h"

using namespace kmod;
CameraListWidget::CameraListWidget(Attribute attr, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CameraListWidget)
{
    ui->setupUi(this);
   
    
    for(const std::shared_ptr<kmod::CameraParameter> it: kmod::g_camerasParam){

        QLabel *imgLabel = new QLabel;
        imgLabel->setFrameShape(QLabel::Box);
        imgLabel->setFrameShadow(QLabel::Raised);
        imgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        imgLabel->setMinimumSize(500, 240);
        imgLabel->setScaledContents(true);

        AspectSingleLayout *aspLayout = new AspectSingleLayout(NULL, it->imageWidthToHeight);
        aspLayout->setContentsMargins(0, 0, 0, 0);
        aspLayout->addWidget(imgLabel);
        aspLayout->setAlignment(imgLabel, Qt::AlignCenter);

        QPushButton *calibButton = new QPushButton("calibration");
        //calibButton->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        calibButton->setMinimumSize(60, 60);
        //calibButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

   

        cv::Mat cvImage;
        if(kmod::g_pDevDatav->mCameras[it->devId]->isConncected()) {
        
           // QImage img;
            std::shared_ptr<kmod::MatTimeStamp>  mymat = kmod::g_ImgbufList[it->devId].pop();
			if (mymat != nullptr && !mymat->imgmat.empty()){		
                
                cvImage = mymat->imgmat.clone();
                QImage img = kmod::MyUtil::MatToQImage(cvImage);
                imgLabel->setPixmap(QPixmap::fromImage(img));
                calibButton->setEnabled(true);
			}
        } else {
             calibButton->setEnabled(false);
        }

        QLabel *calibStatusLabel = new QLabel;
        calibStatusLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

        std::function<void(void)> lfp;

        bool ready(0);
        if(attr == Attribute::intrinsic) {

            ready = it->intrinsicDistReady;
            lfp = [=](){                        
                        CalibrateWidget *calibw = new CalibrateWidget(it, this);
                        calibw->setAttribute(Qt::WA_DeleteOnClose);
                        calibw->setWindowTitle(QString::fromStdString(it->name));//QString::fromStdString(cmrfgpath)
                        calibw->exec();
                        setCalibrateStatus(calibStatusLabel,it, it->intrinsicDistReady);
                    };

        }        
        else if (attr == Attribute::extrinsic) {
            ready = it->extrinsicReady;
            lfp = [=](){                        
                        cv::Mat img;
                        cvImage.copyTo(img);
                        ExtrinsicDialog *exDialog = new ExtrinsicDialog(img, it, this);
                        exDialog->setAttribute(Qt::WA_DeleteOnClose);
                        exDialog->setWindowTitle(QString::fromStdString(it->name));
                        exDialog->exec();
                        setCalibrateStatus(calibStatusLabel, it, it->extrinsicReady);
                    };

            if(!it->intrinsicDistReady) {
                calibButton->setEnabled(false);
            }
        }

       
        setCalibrateStatus(calibStatusLabel, it, ready);

        QVBoxLayout *hlayout = new QVBoxLayout;

        hlayout->addLayout(aspLayout, 5);
        hlayout->addStretch(10);
        hlayout->addWidget(calibStatusLabel, 1);
        hlayout->addWidget(calibButton, 1);
        
        hlayout->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);        
        ui->hLayout->addLayout(hlayout);

        connect(calibButton, &QPushButton::clicked, lfp);
    }
}

CameraListWidget::~CameraListWidget()
{
    delete ui;
}

void CameraListWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    // Q_UNUSED(event);
    // if(isMaximized()) {
    //     showNormal();
    // } else {
    //     showMaximized();
    // }
}

void CameraListWidget::setCalibrateStatus(QLabel *label, const std::shared_ptr<kmod::CameraParameter> &p, bool ready)
{
    QString s = QString::fromStdString(p->name) + ":";
    s += (ready ? "calibrated" : "to be calibrated");
    label->setText(s);
}

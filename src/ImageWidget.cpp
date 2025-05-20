#include "ImageWidget.h"

#include <QPainter>
#include <qdebug.h>
//#include <QMessageBox.h>

class Shape
{
public:
    enum Code { Line, Rect };
    Shape();
    void setStart(QPoint s) {
        start = s;
    }
    void setEnd(QPoint e) {
        end = e;
    }
    QPoint startPoint() {
        return start;
    }
    QPoint endPoint() {
        return end;
    }
    void virtual paint(QPainter& painter) = 0;
protected:
    QPoint start;
    QPoint end;
};
class Line : public Shape
{
public:
    Line();
    void paint(QPainter& painter) {
        painter.drawLine(start, end);
    }
};
class Rect : public Shape
{
public:
    Rect();
    void paint(QPainter& painter) {
        painter.drawRect(start.x(), start.y(),
            end.x() - start.x(), end.y() - start.y());
    }
};

ImageWidget::ImageWidget(QWidget *parent)
	: QWidget(parent)
{
    _selrect = QRect(100, 100, 1000, 600);
}

void ImageWidget::setImage(const QImage & showimage)
{
	this->image = showimage;
	update();
}
void	ImageWidget::saveCurrImage(QString strname)
{
   this->image.save(strname);
}

void ImageWidget::paintEvent(QPaintEvent *)
{
	QPainter pr(this);


    QFont font("Arial", 24, QFont::Normal, false);	
    QPen pen;
    pen.setWidth(5);
    pr.drawImage(rect(), image);   
    pen.setColor(Qt::blue);
    pr.setPen(pen);
    pr.setFont(font);
    if (!_bAllowModify) {
        if (_bshowROI) {           
            pr.drawText(10, 50, QString::number(_ncmrid + 1));
        }        
        return;
    }    
  
/* 

    pen.setColor(Qt::red);
    pen.setStyle(Qt::SolidLine); //���÷��
    pen.setWidth(30* _fscale);
    pr.setPen(pen);

  //  QPoint p(event->pos().x() / _fscale, event->pos().y() / _fscale);


    for (int i = 0; i < _drawPoints.size(); ++i){
        const QPoint& point = _drawPoints.at(i);
        QPoint pnt(point.x()* _fscale, point.y() * _fscale);
        pr.drawEllipse(pnt, int(16 * _fscale), int(16 * _fscale));
        //pr.drawEllipse(p, 20 * _fscale, 20 * _fscale);
        //pr.drawEllipse
    }
    pen.setWidth(5);
    pen.setColor(Qt::green);
    pen.setStyle(Qt::DashLine); //���÷��
    pr.setPen(pen);
    pr.drawRect(_selrect);
    pr.drawText(10, 50, QString::number(_ncmrid + 1));
*/

}
void	ImageWidget::setMaskPoint(const QVector<QPoint>& maskPoint)
{
    _drawPoints.clear();
    for (int i = 0; i < maskPoint.size(); ++i) {
        const QPoint& point = maskPoint.at(i);
        _drawPoints.push_back(point);
    }
    update();

}
QVector<QPoint>	ImageWidget::getMaskPoint()
{
    return _drawPoints;

}
QRect	ImageWidget::getDetectArea() 
{ 
    QRect  rc = QRect(_selrect.x() /_fscale, 
                      _selrect.y() / _fscale,
                      _selrect.width() / _fscale, 
                      _selrect.height() / _fscale );

    return rc;
}
void	ImageWidget::setDetectArea(QRect& rc) 
{
    _selrect = QRect(rc.x() * _fscale, 
                     rc.y() * _fscale,
                     rc.width() * _fscale, 
                     rc.height() * _fscale);
}
void	ImageWidget::setSetScale(float fscale)
{ 
    _fscale = fscale; 
}
void ImageWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        _bMouseDown = true;
    }
    else if (event->button() == Qt::MouseButton::RightButton) {
        _brMouseDown = true;
    }
    //_bAreaModified = false;
    if (_bAllowModify) {
        if (abs(event->pos().x() - _selrect.x()) < 5)
        {
            _rectStatus = 0;
        }
        else if (abs(event->pos().y() - _selrect.y()) < 5)
        {
            _rectStatus = 1;
        }
        else if (abs(event->pos().x() - _selrect.right()) < 5)
        {
            _rectStatus = 2;
        }
        else if (abs(event->pos().y() - _selrect.bottom()) < 5)
        {
            _rectStatus = 3;
        }
        else
        {
            _rectStatus = -1;
        }
    }   
    
}
void ImageWidget::mouseMoveEvent(QMouseEvent* event)
{

    if (_bAllowModify) {

        if (_bMouseDown) {

            if (event->pos().x() > 1 && event->pos().y() > 1 &&
                event->pos().x() < rect().width() - 1 && event->pos().y() < rect().height() - 1) {

                if (_rectStatus == 0 && event->pos().x() < _selrect.right())
                {
                    _selrect.setX(event->pos().x());

                }
                else if (_rectStatus == 1 && event->pos().y() < _selrect.bottom())
                {
                    _selrect.setY(event->pos().y());
                }
                else if (_rectStatus == 2 && event->pos().x() > _selrect.x())
                {
                    _selrect.setRight(event->pos().x());
                }
                else if (_rectStatus == 3 && event->pos().y() > _selrect.y())
                {
                    _selrect.setBottom(event->pos().y());
                }
                else {
                    //����ԭʼ�Ĵ�С
                    QPoint p(event->pos().x()/ _fscale, event->pos().y() / _fscale);                     
                    bool bexist = false;
                    // for each (QPoint myp in _drawPoints){
                    //     //if(  myp.x()-p.x() )
                    //     if (sqrt((myp.x() - p.x()) * (myp.x() - p.x()) + (myp.y() - p.y()) * (myp.y() - p.y())) < 15 ){
                    //         bexist = true;
                    //         break;
                    //     }

                    // }
                    if (!bexist) {
                        _drawPoints.push_back(p);
                    }
                   //
                }

            }


        }else if (_brMouseDown){
            if (event->pos().x() > 1 && event->pos().y() > 1 &&
                event->pos().x() < rect().width() - 1 && event->pos().y() < rect().height() - 1) {

                QPoint p(event->pos().x() / _fscale, event->pos().y() / _fscale);

               // QVector<QPoint>::iterator ite = _drawPoints.begin();
                // for (auto ite = _drawPoints.begin(); ite != _drawPoints.end(); ) {
                   
                //     if (sqrt((ite->x() - p.x()) * (ite->x() - p.x()) + 
                //              (ite->y() - p.y()) * (ite->y() - p.y())) < 25) {
                //         ite =  _drawPoints.erase(ite);
                //        // qDebug() << _drawPoints.size();
                //     }
                //     else {
                //         ite++;
                //     }
                   
                   
                // }
                //bool bexist = false;
                //for each (QPoint myp in _drawPoints) {
                //    //if(  myp.x()-p.x() )
                //    if (sqrt((myp.x() - p.x()) * (myp.x() - p.x()) + (myp.y() - p.y()) * (myp.y() - p.y())) < 2) {
                //         _drawPoints.removeOne(myp);
                //    }

                //}
               /* if (!bexist) {
                    _drawPoints.push_back(p);
                }*/


            }
        }
        else {
            if (abs(event->pos().x() - _selrect.x()) < 5)
            {
                _rectStatus = 0;
                this->setCursor(Qt::SizeHorCursor);
                //qDebug() << "(Qt::SizeHorCursor)";
            }
            else if (abs(event->pos().y() - _selrect.y()) < 5)
            {
                _rectStatus = 1;
                this->setCursor(Qt::SizeVerCursor);
            }
            else if (abs(event->pos().x() - _selrect.right()) < 5)
            {
                _rectStatus = 2;
                this->setCursor(Qt::SizeHorCursor);
            }
            else if (abs(event->pos().y() - _selrect.bottom()) < 5)
            {
                _rectStatus = 3;
                this->setCursor(Qt::SizeVerCursor);
            }
            else
            {
                _rectStatus = -1;
                this->setCursor(Qt::ArrowCursor);
            }
        }


        // qDebug() << "mouseMoveEvent";

        //if (_bMouseDown) {

        //    if (event->pos().x() > 3 && event->pos().y() > 3 &&
        //        event->pos().x() < rect().width() - 3 && event->pos().y() < rect().height() - 3) {

        //        if (_rectStatus == 0 && event->pos().x() < _selrect.right())
        //        {
        //            _selrect.setX(event->pos().x());
        //        }
        //        else if (_rectStatus == 1 && event->pos().y() < _selrect.bottom())
        //        {
        //            _selrect.setY(event->pos().y());
        //        }
        //        else if (_rectStatus == 2 && event->pos().x() > _selrect.x())
        //        {
        //            _selrect.setRight(event->pos().x());
        //        }
        //        else if (_rectStatus == 3 && event->pos().y() > _selrect.y())
        //        {
        //            _selrect.setBottom(event->pos().y());
        //        }

        //    }




        //}
        //else {
        //    if (abs(event->pos().x() - _selrect.x()) < 5)
        //    {
        //        _rectStatus = 0;
        //        this->setCursor(Qt::SizeHorCursor);
        //        //qDebug() << "(Qt::SizeHorCursor)";
        //    }
        //    else if (abs(event->pos().y() - _selrect.y()) < 5)
        //    {
        //        _rectStatus = 1;
        //        this->setCursor(Qt::SizeVerCursor);
        //    }
        //    else if (abs(event->pos().x() - _selrect.right()) < 5)
        //    {
        //        _rectStatus = 2;
        //        this->setCursor(Qt::SizeHorCursor);
        //    }
        //    else if (abs(event->pos().y() - _selrect.bottom()) < 5)
        //    {
        //        _rectStatus = 3;
        //        this->setCursor(Qt::SizeVerCursor);
        //    }
        //    else
        //    {
        //        _rectStatus = -1;
        //        this->setCursor(Qt::ArrowCursor);
        //    }

        //}


    }
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        _bMouseDown = false;
       

    }
    else if (event->button() == Qt::MouseButton::RightButton) {
        _brMouseDown = false;
    }
    _bAreaModified = true;
   
}
void ImageWidget::mouseDoubleClickEvent(QMouseEvent* event)
{

    if (event->button() == Qt::MouseButton::RightButton){

        if (_bAllowModify) {

                ;
            // switch (QMessageBox::information(this, tr("Area"),
            //     tr("Do you really want to Reset?"),
            //     tr("Yes"), tr("No"), 0, 1)) {
            // case 0:
            //     event->accept();
            //     _selrect = QRect(100, 100, 1000, 600);
            //     _drawPoints.clear();
            //     break;
            // case 1:
            // default:
            //     event->ignore();
            //     break;
            // }
        }

       

    }else if (event->button() == Qt::MouseButton::LeftButton) {
        _bshowROI = !_bshowROI;
    }
    _bAreaModified = true;
}
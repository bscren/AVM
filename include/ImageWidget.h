/*
author: chenjinbo  date:2022/10/05
*/
#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QImage>
#include <qrect.h>
#include <QMouseEvent>
#include <QVector>

class Shape;
class Line;
class Rect;

class ImageWidget : public QWidget
{
	Q_OBJECT
private:
	QImage image;
	QVector<QPoint>	_drawPoints;
public:
	explicit ImageWidget(QWidget *parent = 0);
	void setImage(QImage const &showimage);
	QImage getImage() const{
		return image;
	}
	void setDetectAreaFlag(bool bmodify) {
		_bAllowModify = bmodify;
	}
	void clearAreaModifiedFlag() {
		_bAreaModified = false;
	}
	bool  getAreaModifiedFlag() {
		return _bAreaModified;
	}
	QRect	getDetectArea();
	void	setDetectArea(QRect& rc);
	void	setSetScale(float fscale);
	void	setCameraID(int nid) { _ncmrid = nid; }
	void	saveCurrImage(QString strname);
	void	setMaskPoint(const QVector<QPoint> & maskPoint);
	QVector<QPoint>	getMaskPoint();

signals:

public slots:
	// QWidget interface
protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent* );
	void mouseReleaseEvent(QMouseEvent* );
	void mouseDoubleClickEvent(QMouseEvent* event);
private:
	QRect		_selrect;
	int			_rectStatus = -1;
	bool		_bMouseDown = false;
	bool		_brMouseDown = false;
	bool		_bAllowModify = false;//修改模式
	bool		_bAreaModified = false;//区域修改
	bool		_bshowROI = false;//区域显示
	float		_fscale = 1.0;
	int			_ncmrid = 0;
	
};

#endif // IMAGEWIDGET_H

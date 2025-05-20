#pragma once

#include <QtWidgets/QMainWindow>
#include <QDialog>
#include <QMap>
#include "ui_about.h"

class aboutdlg:public QDialog
{
	Q_OBJECT
public:
    aboutdlg(QDialog* parent = nullptr);
    ~aboutdlg();

private:
    Ui::AboutDialog ui;

};


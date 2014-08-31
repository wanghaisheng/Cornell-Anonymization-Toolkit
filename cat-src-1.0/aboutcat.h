#ifndef ABOUTCAT_H
#define ABOUTCAT_H

#include <QtGui>
#include "ui_aboutcat.h"


class myAboutCAT : public QWidget
{
	Q_OBJECT

		friend class myMainWindow;

public:
	myAboutCAT(QWidget *parent = 0);
	~myAboutCAT()
	{
		this->close();
	}

private:
	Ui::myAboutCATClass ui;

};


#endif
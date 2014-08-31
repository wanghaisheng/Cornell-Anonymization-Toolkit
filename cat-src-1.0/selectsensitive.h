#ifndef SELECTSENSITIVE_H
#define SELECTSENSITIVE_H

#include <QtGui>
#include "ui_selectsensitive.h"
#include "Microdata.h"


class mySelectSensitive : public QWidget
{
	Q_OBJECT

public:
	mySelectSensitive(QWidget *parent = 0);
	void loadMeta(const vector<Metadata> &);

public slots:
	void okay();
	
signals:
	void signalBack(const int);

private:
	Ui::mySelectsensitiveClass ui; 

};


#endif
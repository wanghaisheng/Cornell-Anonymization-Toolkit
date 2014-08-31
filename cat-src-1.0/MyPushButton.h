#ifndef MYPUSHBUTTON_H
#define MYPUSHBUTTON_H

#include <QtGui>

class myPushButton : public QPushButton
{
	Q_OBJECT

public slots:
	void setMyDisabled( bool );	
};


#endif
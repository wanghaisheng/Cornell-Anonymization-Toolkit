#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include <QtGui>

class myLineEdit : public QLineEdit
{
	Q_OBJECT

public slots:
	void getHierarchy();
	void setMyDisabled( bool );
	void setMyClear( bool );
};


#endif
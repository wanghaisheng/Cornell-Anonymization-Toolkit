#ifndef MYCHECKBOX_H
#define MYCHECKBOX_H

#include <QtGui>

class myCheckBox : public QCheckBox
{
	Q_OBJECT

		public slots:
			void setMyChecked( bool );	
};


#endif
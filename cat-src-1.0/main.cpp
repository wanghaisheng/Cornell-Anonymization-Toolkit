#include <QtGui/QApplication>
#include "mymainwindow.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setStyle( "cleanlooks" );
	myMainWindow w;
	QCursor myCursor( Qt::ArrowCursor );
	w.setCursor( myCursor );
	w.show();
	return a.exec();
}

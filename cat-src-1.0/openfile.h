#ifndef OPENFILE_H
#define OPENFILE_H

#include <QtGui>
#include "ui_openfile.h"


class myOpenFile : public QWidget
{
	Q_OBJECT

friend class myMainWindow;

public:
	myOpenFile(QWidget *parent = 0);
	~myOpenFile()
	{
		this->close();
	}

public slots:
	void getPathMicro();	// Get micro data file path
	void getPathMeta();		// Get meta data file path
	void okay();			
	void clear();

signals:
    void signalBack( const QString&, const QString&);	// Signal the main window with micro and meta data file path

private:
	Ui::myOpenfileClass ui;

};


#endif
#include "openfile.h"

// including <QtGui> saves us to include every class user, <QString>, <QFileDialog>,...

myOpenFile::myOpenFile(QWidget *parent)
{
	ui.setupUi(this); // this sets up GUI

	// signals/slots mechanism in action
	connect( ui.pushButton_micro_browse, SIGNAL( clicked() ), this, SLOT( getPathMicro() ) ); 
	connect( ui.pushButton_meta_browse, SIGNAL( clicked() ), this, SLOT( getPathMeta() ) ); 
	connect( ui.pushButton_clear, SIGNAL( clicked() ), this, SLOT( clear() ) );
	connect( ui.pushButton_ok, SIGNAL( clicked() ), this, SLOT( okay() ) );
	this->setWindowModality( Qt::ApplicationModal );
}


void myOpenFile::getPathMicro()
{
	QString path;
	
	path = QFileDialog::getOpenFileName(
		this,
		"Open",
		QString::null,
		QString::null);

	ui.lineEdit_micro->setText( path );
}

void myOpenFile::getPathMeta()
{
	QString path;
	
	path = QFileDialog::getOpenFileName(
		this,
		"Open",
		QString::null,
		QString::null);

	ui.lineEdit_meta->setText( path );
}


void myOpenFile::clear()
{
	ui.lineEdit_micro->clear();	
	ui.lineEdit_meta->clear();
}

void myOpenFile::okay()
{

	/* Add code to forward the lineEdit value to the Anonymizer */

	/* Below is test code: read the file and change the label content*/

	this->setCursor( Qt::WaitCursor );

	emit signalBack( ui.lineEdit_micro->text(), ui.lineEdit_meta->text());

	this->close();

	this->setCursor( Qt::ArrowCursor );
}

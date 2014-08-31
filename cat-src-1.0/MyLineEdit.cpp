#include "MyLineEdit.h"


void myLineEdit::getHierarchy()
{
	QString path;
	
	path = QFileDialog::getOpenFileName(
		this,
		"Choose a file to open",
		QString::null,
		QString::null);

	this->setText( path );
}

void myLineEdit::setMyDisabled(bool checked)
{
	if(checked)
		this->setDisabled(true);
}

void myLineEdit::setMyClear(bool checked)
{
	if(checked)
		this->clear();
}
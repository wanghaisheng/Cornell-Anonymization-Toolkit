#ifndef SHOWGENERLIZATION_H
#define SHOWGENERLIZATION_H

#include <QtGui>
#include "ui_showgeneralization.h"
#include "Generalization.h"

class myShowGeneralization : public QWidget
{
	Q_OBJECT

		friend class myMainWindow;

public:
	myShowGeneralization(QWidget *parent = 0);
	~myShowGeneralization()
	{
		this->close();
	}

public slots:
	void okay();	// Close this sub-window
	void GeneralizationInfo( int index );	// Show generalization information of certain attribute by index
	void loadGen( Generalization& gen );	// Load Generalization information from main window

private:
	Ui::myOpengeneralizationClass ui;		
	Generalization* gen_info;

};

#endif
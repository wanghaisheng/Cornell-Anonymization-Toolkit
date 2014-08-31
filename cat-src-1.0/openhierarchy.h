#ifndef OPENHIERARCHY_H
#define OPENHIERARCHY_H

#include <QtGui>
#include "ui_openhierarchy.h"
#include "Microdata.h"


class myOpenHierarchy : public QWidget
{
	Q_OBJECT

friend class myMainWindow;

public:
	myOpenHierarchy(QWidget *parent = 0);
	~myOpenHierarchy()
	{
		this->close();
	}

	void loadMeta(const vector<Metadata> &);	// Load meta data from the main window

public slots:
	void getPathHierarchy();					// Load hierarchy files' path for all attributes
	void okay();								// Load hierarchy file names for all QI attributes
	void clear();								// Clear hierarchy file names for all QI attributes
	
signals:
	void signalBack(const QTableWidget *);		// Signal the main window after setting hierarchy file names

private:
	Ui::myOpenhierarchyClass ui; 

};


#endif
#ifndef OPENLOG_H
#define OPENLOG_H

#include <QtGui>
#include "ui_openlog.h"
#include "mymainwindow.h"


class myOpenLog : public QWidget
{
	Q_OBJECT

friend class myMainWindow;

public:
	myOpenLog(QWidget *parent = 0);
	~myOpenLog()
	{
		this->close();
	}

	void UpdateLog(const class myLog&);	// Insert new record into the user log

public slots:
	void clear();					// Clear the selection of the log record
	void undo();					// Undo user log records from the selected record to the newest record
	void redo();					// Redo user log records to the selected record
	void selectState(int, int);		// Select the log record for undo or redo

signals:
	void signalBack(const int);		// Signal to the main window with the selected record

private:
	Ui::myOpenlogClass ui;

	int select_state;		// Selected state
	int current_state;		// Current state

};


#endif
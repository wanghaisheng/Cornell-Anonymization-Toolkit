#include <fstream>
#include <iostream>
#include <sstream>
#include "openhierarchy.h"
#include "MyCheckBox.h"
#include "MyLineEdit.h"
#include "MyPushButton.h"
#include "AnonyException.h"

// including <QtGui> saves us to include every class user, <QString>, <QFileDialog>,...

myOpenHierarchy::myOpenHierarchy(QWidget *parent)
{
	ui.setupUi(this); // this sets up GUI

	// signals/slots mechanism in action
	connect( ui.pushButton_ok, SIGNAL( clicked() ), this, SLOT( okay() ) );
	connect( ui.pushButton_hierarchy_browse, SIGNAL( clicked() ), this, SLOT( getPathHierarchy() ) );
	this->setWindowModality( Qt::ApplicationModal );
}

void myOpenHierarchy::loadMeta(const vector<Metadata> &meta)
{
	ui.tableWidget_qi->clearContents();
	ui.tableWidget_qi->setColumnCount( 5 );
	ui.tableWidget_qi->setRowCount( meta.size() );
	
	int indexRow;
	QStringList temp_list;
	temp_list << "Attribute" << " SA " << " QI " << "  Quasi-Identifier's Hierarchy File  " << "Choose..";

	ui.tableWidget_qi->setHorizontalHeaderLabels( temp_list );

	QTableWidgetItem* p_item;
	myCheckBox *cbox;
	myCheckBox *cbox2;
	myLineEdit *ledit;
	myPushButton *pbutton;
	for(indexRow = 0; indexRow < meta.size(); indexRow++){		

		//set first column: attribute name
		p_item = new QTableWidgetItem;
		string value = meta[indexRow].m_strName;
		p_item->setText( value.c_str() );
		ui.tableWidget_qi->setItem( indexRow, 0, p_item );

		//set second column: CheckBox, third column: LineEdit, forth column: PushButton
		cbox = new myCheckBox();
		cbox2 = new myCheckBox();
		ledit = new myLineEdit();
		pbutton = new myPushButton();

		ui.tableWidget_qi->setCellWidget( indexRow, 1, cbox2 );
		ui.tableWidget_qi->setCellWidget( indexRow, 2, cbox );
		ledit->setDisabled(true);
		ui.tableWidget_qi->setCellWidget( indexRow, 3, ledit );
		pbutton->setText("Browse..");
		pbutton->setDisabled(true);
		ui.tableWidget_qi->setCellWidget( indexRow, 4, pbutton );

		//connect checkbox QI to enable lineedit and pushbutton
		connect( ui.tableWidget_qi->cellWidget( indexRow, 2 ), SIGNAL( clicked( bool ) ), ui.tableWidget_qi->cellWidget( indexRow, 3 ), SLOT( setEnabled( bool ) ) );
		connect( ui.tableWidget_qi->cellWidget( indexRow, 2 ), SIGNAL( clicked( bool ) ), ui.tableWidget_qi->cellWidget( indexRow, 4 ), SLOT( setEnabled( bool ) ) );

		//connect checkbox QI to disable checkbox SA
		connect( ui.tableWidget_qi->cellWidget( indexRow, 2 ), SIGNAL( clicked( bool ) ), ui.tableWidget_qi->cellWidget( indexRow, 1 ), SLOT( setMyChecked( bool ) ) );

		//connect pushbutton to get hierarchy file
		connect( ui.tableWidget_qi->cellWidget( indexRow, 4 ), SIGNAL( clicked() ), ui.tableWidget_qi->cellWidget( indexRow, 3 ), SLOT( getHierarchy() ) );

		//connect checkbox SA to disable checkbox QI, lineedit, pushbutton at the same line;
		connect( ui.tableWidget_qi->cellWidget( indexRow, 1 ), SIGNAL( clicked( bool ) ), ui.tableWidget_qi->cellWidget( indexRow, 2 ), SLOT( setMyChecked( bool ) ) );
		connect( ui.tableWidget_qi->cellWidget( indexRow, 1 ), SIGNAL( clicked( bool ) ), ui.tableWidget_qi->cellWidget( indexRow, 3 ), SLOT( setMyClear( bool ) ) );
		connect( ui.tableWidget_qi->cellWidget( indexRow, 1 ), SIGNAL( clicked( bool ) ), ui.tableWidget_qi->cellWidget( indexRow, 3 ), SLOT( setMyDisabled( bool ) ) );
		connect( ui.tableWidget_qi->cellWidget( indexRow, 1 ), SIGNAL( clicked( bool ) ), ui.tableWidget_qi->cellWidget( indexRow, 4 ), SLOT( setMyDisabled( bool ) ) );
	}

	//connect sa checkbox to exclude each other
	for(indexRow = 0; indexRow < meta.size(); indexRow++){		

		int indexRow2;
		for(indexRow2 = 0; indexRow2 < meta.size(); indexRow2++){

			if(indexRow == indexRow2)
				continue;

			connect( ui.tableWidget_qi->cellWidget( indexRow, 1 ), SIGNAL( clicked( bool ) ), ui.tableWidget_qi->cellWidget( indexRow2, 1 ), SLOT( setMyChecked( bool ) ) );
		}
	}

	//connect pushbutton_clear to clear all
	connect( ui.pushButton_clear, SIGNAL( clicked() ), this, SLOT( clear() ) );

	ui.tableWidget_qi->resizeColumnsToContents();
}

void myOpenHierarchy::clear()
{
	int indexRow;
	for(indexRow = 0; indexRow < ui.tableWidget_qi->columnCount(); indexRow++){		

		((myCheckBox *) ui.tableWidget_qi->cellWidget( indexRow, 1 ))->setChecked(false);
		((myCheckBox *) ui.tableWidget_qi->cellWidget( indexRow, 2 ))->setChecked(false);
		((myLineEdit *) ui.tableWidget_qi->cellWidget( indexRow, 3 ))->clear();
		((myLineEdit *) ui.tableWidget_qi->cellWidget( indexRow, 3 ))->setDisabled(true);
		((QPushButton *) ui.tableWidget_qi->cellWidget( indexRow, 4 ))->setDisabled(true);
	}
}

void myOpenHierarchy::okay()
{
	bool error1 = false, error2 = true;
	
	int indexRow;
	for(indexRow = 0; indexRow < ui.tableWidget_qi->rowCount(); indexRow++){

		if(((QCheckBox *) ui.tableWidget_qi->cellWidget(indexRow, 2))->isChecked()){

			if(((myLineEdit *) ui.tableWidget_qi->cellWidget(indexRow, 3))->text() == ""){

				error1 = true;
			}
		}
		if(((QCheckBox *) ui.tableWidget_qi->cellWidget(indexRow, 1))->isChecked()){

			error2 = false;
		}
	}

	if(error1){

		QMessageBox::information(this, "Message", "Please choose a hierarchy file for each selected quasi-identifier.");
	}
	else if(error2){

		QMessageBox::information(this, "Message", "Please choose a sensitive attribute.");
	}
	else {

		emit signalBack(ui.tableWidget_qi);
		this->close();
	}
}

void myOpenHierarchy::getPathHierarchy()
{
	QString path, qline;

	path = QFileDialog::getOpenFileName(
		this,
		"Open",
		QString::null,
		QString::null);

	ui.lineEdit_hierarchy->setText( path );

	ifstream hierarchy_file( path.toLatin1().data() );

	try
	{
		AnonyException temp_exception;
		stringstream temp_sstream;
		if (hierarchy_file.is_open() == false)
		{
			temp_sstream << "HierarchyData::ReadFiles Error: Cannot Open HierarchyPath File: " << path.toLatin1().data();
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}

		string line;

		for ( int indexRow = 0;indexRow < ui.tableWidget_qi->rowCount(); indexRow++ )
		{
			getline( hierarchy_file, line );
			qline = QString( line.c_str() );

			((myLineEdit *) ui.tableWidget_qi->cellWidget(indexRow, 3))->setText( qline );
		}                            

		hierarchy_file.close();
	}
	catch( AnonyException e )
	{
		if (hierarchy_file.is_open() == true)
		{
			hierarchy_file.close();
		}
		ui.tableWidget_qi->clearContents();
		ui.tableWidget_qi->setRowCount( 0 );

		cout << e.GetMsg() << endl;

		throw( e );
		return;
	}
}
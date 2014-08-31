#include "selectsensitive.h"
#include "MyLineEdit.h"

// including <QtGui> saves us to include every class user, <QString>, <QFileDialog>,...

mySelectSensitive::mySelectSensitive(QWidget *parent)
{
	ui.setupUi(this); // this sets up GUI

	// signals/slots mechanism in action
	connect( ui.pushButton_ok, SIGNAL( clicked() ), this, SLOT( okay() ) );
}

void mySelectSensitive::loadMeta(const vector<Metadata> &meta)
{

	ui.tableWidget_qi->setColumnCount( 2 );
	ui.tableWidget_qi->setRowCount( meta.size() );
	
	int indexRow;
	QStringList temp_list;
	temp_list << "Attribute" << "Sensitive";

	ui.tableWidget_qi->setHorizontalHeaderLabels( temp_list );

	QTableWidgetItem* p_item;
	QCheckBox *cbox;
	for(indexRow = 0; indexRow < meta.size(); indexRow++){		

		//set first column: attribute name
		p_item = new QTableWidgetItem;
		string value = meta[indexRow].m_strName;
		p_item->setText( value.c_str() );
		ui.tableWidget_qi->setItem( indexRow, 0, p_item );

		//set second column: CheckBox, third column: LineEdit, forth column: PushButton
		cbox = new QCheckBox();

		ui.tableWidget_qi->setCellWidget( indexRow, 1, cbox );

		//connect pushbutton_clear to clear all lineedits
		connect( ui.pushButton_clear, SIGNAL( clicked( bool ) ), ui.tableWidget_qi->cellWidget( indexRow, 1 ), SLOT( setChecked ( bool ) ) );
		connect( ui.pushButton_clear, SIGNAL( clicked( bool ) ), ui.tableWidget_qi->cellWidget( indexRow, 1 ), SLOT( setDisabled( bool ) ) );
	}

	for(indexRow = 0; indexRow < meta.size(); indexRow++){		

		int indexRow2;
		for(indexRow2 = 0; indexRow2 < meta.size(); indexRow2++){

			if(indexRow == indexRow2)
				continue;

			//connect pushbutton_clear to clear all lineedits
			connect( ui.tableWidget_qi->cellWidget( indexRow, 1 ), SIGNAL( clicked( bool ) ), ui.tableWidget_qi->cellWidget( indexRow2, 1 ), SLOT( setDisabled( bool ) ) );
		}
	}


	ui.tableWidget_qi->resizeColumnsToContents();
}

void mySelectSensitive::okay()
{
	bool error = true;
	
	int indexRow, sensRow;
	for(indexRow = 0; indexRow < ui.tableWidget_qi->rowCount(); indexRow++){

		if(((QCheckBox *) ui.tableWidget_qi->cellWidget(indexRow, 1))->isChecked()){

			error = false;
			sensRow = indexRow;
		}
	}

	if(error){

		QMessageBox::information(this, "Message", "Please choose an attribute as the sensitive attribute.");
	}
	else {

		emit signalBack(sensRow);
		this->close();
	}
}


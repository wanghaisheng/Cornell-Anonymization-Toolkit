#include "openlog.h"
#include "MyCheckBox.h"
#include "MyLineEdit.h"
#include "MyPushButton.h"

// including <QtGui> saves us to include every class user, <QString>, <QFileDialog>,...

myOpenLog::myOpenLog(QWidget *parent)
{
	ui.setupUi(this); // this sets up GUI

	ui.tableWidget_log->setSelectionMode ( QAbstractItemView::MultiSelection );
	ui.tableWidget_log->setEditTriggers( QAbstractItemView::NoEditTriggers );
	this->setWindowModality( Qt::ApplicationModal );

	// signals/slots mechanism in action
	connect( ui.pushButton_redo, SIGNAL( clicked() ), this, SLOT( redo() ) );
	connect( ui.pushButton_undo, SIGNAL( clicked() ), this, SLOT( undo() ) );
	connect( ui.tableWidget_log, SIGNAL( cellPressed( int, int ) ), this, SLOT( selectState( int, int ) ) );

	ui.pushButton_redo->setDisabled( true );
	ui.pushButton_undo->setDisabled( true );

	// setup the table for showing log
	ui.tableWidget_log->setColumnCount( 2 );
	ui.tableWidget_log->setRowCount( 0 );

	QStringList temp_list;
	temp_list << " Action " << " Time ";
	ui.tableWidget_log->setHorizontalHeaderLabels( temp_list );

	select_state = -1;
	current_state = -1;
}

void myOpenLog::selectState(int row, int column)
{
	ui.tableWidget_log->clearSelection();

	if ( column == 0)
	{
		// select "undo" actions
		if ( row <= current_state )
		{
			for ( int index = row; index <= current_state; index++ )
			{
				ui.tableWidget_log->setCurrentCell( index, 0 );
				ui.tableWidget_log->setCurrentCell( index, 1 );
			}

			select_state = row - 1;
			ui.pushButton_redo->setDisabled( true );
			ui.pushButton_undo->setDisabled( false );

		}
		// select "redo" actions
		else
		{
			for ( int index = current_state + 1; index <= row; index++ )
			{
				ui.tableWidget_log->setCurrentCell( index, 0 );
				ui.tableWidget_log->setCurrentCell( index, 1 );
			}

			select_state = row;
			ui.pushButton_redo->setDisabled( false );
			ui.pushButton_undo->setDisabled( true );
		}
	}
	else
	{
		select_state = -1;
	}
}

void myOpenLog::UpdateLog(const myLog &newLog)
{
	current_state++;

	QTableWidgetItem* p_item = new QTableWidgetItem();
	int old_row_count = ui.tableWidget_log->rowCount();

	ui.tableWidget_log->setRowCount( old_row_count + 1);
	if ( newLog.type_log == anony::GENERALIZATION )
	{
		if ( newLog.type_verifier == anony::DIVERSITY )
		{
			p_item->setText( QString("Generalize with Diversity using l = " + QString::number(newLog.para_l) + ", c = " + QString::number(newLog.para_c) ) );
		}
		else if ( newLog.type_verifier == anony::CLOSENESS )
		{
			p_item->setText( QString("Generalize with Closeness using t = " + QString::number(newLog.para_t) ) );
		}
	}
	else if ( newLog.type_log == anony::DELETE )
	{
		p_item->setText( QString("Delete with threshold = " + QString::number(newLog.para_num_del) ) );
	}
	else
	{
		if ( newLog.type_verifier == anony::DIVERSITY )
		{
			p_item->setText( QString("Evaluate risk using Diversity Model with " + QString::number(newLog.para_vec_bg_qi.size()) + " QIs and " + QString::number(newLog.para_piece_sa) + " pieces of SA" ) );
		}
		else if ( newLog.type_verifier == anony::CLOSENESS )
		{
			p_item->setText( QString("Evaluate risk using Closeness Model with " + QString::number(newLog.para_vec_bg_qi.size()) + " QIs" ) );
		}
	}
	ui.tableWidget_log->setItem( old_row_count, 0, p_item);

	p_item = new QTableWidgetItem();
	p_item->setText( newLog.timestamp.toString() );
	ui.tableWidget_log->setItem( old_row_count, 1, p_item);

	ui.tableWidget_log->resizeColumnsToContents();
}

void myOpenLog::clear()
{
	ui.tableWidget_log->clearContents();
	ui.tableWidget_log->setRowCount( 0 );
	select_state = -1;
	current_state = -1;
}

void myOpenLog::redo()
{
	QBrush brush_black;
	brush_black.setColor( Qt::black);
	for(int indexRow = current_state + 1; indexRow <= select_state; indexRow++){

		ui.tableWidget_log->item( indexRow, 0 )->setForeground( brush_black );
		ui.tableWidget_log->item( indexRow, 1 )->setForeground( brush_black );
	}

	current_state = select_state;
	emit signalBack( select_state );
	ui.pushButton_undo->setDisabled( false );
	if ( current_state == ui.tableWidget_log->rowCount() - 1 )
	{
		ui.pushButton_redo->setDisabled( true );
	}

	ui.tableWidget_log->clearSelection();
	select_state = -1;
}

void myOpenLog::undo()
{
	QBrush brush_gray;
	brush_gray.setColor( Qt::gray);
	for(int indexRow = select_state + 1; indexRow <= current_state; indexRow++){
	
		ui.tableWidget_log->item( indexRow, 0 )->setForeground( brush_gray );
		ui.tableWidget_log->item( indexRow, 1 )->setForeground( brush_gray );
	}
	
	current_state = select_state;
	emit signalBack( select_state );
	ui.pushButton_redo->setDisabled( false );
	if ( current_state == -1 )
	{
		ui.pushButton_undo->setDisabled( true );
	}

	ui.tableWidget_log->clearSelection();
	select_state = -1;
}
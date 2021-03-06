#include "mymainwindow.h"
#include "chisquaredistr.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

myMainWindow::myMainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this); // this sets up GUI

	setStyle(new QCleanlooksStyle);

	// initialize sub-windows
	openFile = new myOpenFile(this);
	openHierarchy = new myOpenHierarchy(this);
	openLog = new myOpenLog(this);
	showMeta = new myShowMeta(this);
	showGen = new myShowGeneralization(this);
	aboutCAT = new myAboutCAT(this);

	// initialize sorting index to null, order to descending
	sortIndex = -1;
	ascend = false;

	/* --- functions for right panel --- */

	// sliders for setting parameter l, c, t, pieces of background knowledge
	ui.horizontalSlider_l->setRange(0, 99);
    ui.horizontalSlider_l->setValue(0);
	ui.horizontalSlider_c->setRange(0, 9900);
    ui.horizontalSlider_c->setValue(0);
	ui.horizontalSlider_t->setRange(0, 1000);
	ui.horizontalSlider_t->setValue(0);
	ui.spinBox_l->setValue(0);
	ui.spinBox_l->setRange(0, 99);
	ui.doubleSpinBox_c->setValue(0.0);
	ui.doubleSpinBox_c->setRange(0.0, 99.0);
	ui.doubleSpinBox_c->setSingleStep(0.01);
	ui.doubleSpinBox_t->setValue(0.0);
	ui.doubleSpinBox_t->setRange(0.0, 1.0);
	ui.doubleSpinBox_t->setSingleStep(0.001);

	connect( ui.horizontalSlider_l, SIGNAL(valueChanged(int)), ui.spinBox_l, SLOT(setValue(int) ) );
	connect( ui.horizontalSlider_c, SIGNAL(valueChanged(int)), this, SLOT(setInt2Double_c(int) ) );
	connect( ui.horizontalSlider_t, SIGNAL(valueChanged(int)), this, SLOT(setInt2Double_t(int) ) );
	connect( ui.spinBox_l, SIGNAL(valueChanged(int)), ui.horizontalSlider_l, SLOT(setValue(int) ) );
	connect( ui.doubleSpinBox_c, SIGNAL(valueChanged(double)), this, SLOT(setDouble2Int_c(double) ) );
	connect( ui.doubleSpinBox_t, SIGNAL(valueChanged(double)), this, SLOT(setDouble2Int_t(double) ) );
	connect( ui.horizontalSlider_bk, SIGNAL(valueChanged(int)), ui.spinBox_bk, SLOT(setValue(int) ) );
	connect( ui.spinBox_bk, SIGNAL(valueChanged(int)), ui.horizontalSlider_bk, SLOT(setValue(int) ) );

	// buttons for generalize, evaluate risk
	connect( ui.pushButton_generalize, SIGNAL( clicked() ), this, SLOT( generalize() ) );
	connect( ui.pushButton_evaluaterisk, SIGNAL( clicked() ), this, SLOT( evaluateRisk() ) );
	connect( ui.tabWidget_verifier, SIGNAL(currentChanged(int)), this, SLOT( setCurrentVerifier( int ) ));

	// sliders for setting deleting threshold
	ui.doubleSpinBox_threshold->setRange(0, 1.0);
	ui.doubleSpinBox_threshold->setSingleStep(0.01);
	ui.horizontalSlider_threshold->setRange(0, 100);
	ui.horizontalSlider_threshold->setSingleStep( 1 );

	connect( ui.horizontalSlider_threshold, SIGNAL(valueChanged(int)), this, SLOT( setInt2Double_th(int) ) );
	connect( ui.doubleSpinBox_threshold, SIGNAL(valueChanged(double)), this, SLOT( setDouble2Int_th(double) ) );
	connect( ui.doubleSpinBox_threshold, SIGNAL(valueChanged(double)), this, SLOT( changeThreshold(double) ) );
	connect( ui.doubleSpinBox_threshold, SIGNAL(valueChanged(double)), this, SLOT( numTuple(double) ) );
	connect( ui.pushButton_delete, SIGNAL( clicked() ), this, SLOT( deleteTuple() ) );
	connect( ui.horizontalSlider_threshold, SIGNAL( sliderReleased() ), this, SLOT( numTuple() ) );

	// sliders for setting rescaling value
	ui.doubleSpinBox_rescale->setRange(0, 1.0);
	ui.doubleSpinBox_rescale->setSingleStep(0.01);
	ui.horizontalSlider_rescale->setRange(0, 100);
	ui.horizontalSlider_rescale->setSingleStep( 1 );

	connect( ui.horizontalSlider_rescale, SIGNAL(valueChanged(int)), this, SLOT( setInt2Double_r(int) ) );
	connect( ui.doubleSpinBox_rescale, SIGNAL(valueChanged(double)), this, SLOT( setDouble2Int_r(double) ) );
	connect( ui.pushButton_rescale, SIGNAL( clicked() ), this, SLOT( rescaleRisk() ) );

	/* --- functions for left upper panel --- */

	// buttons for next/last page in the table
	connect( ui.pushButton_next, SIGNAL(clicked()), this, SLOT( nextPage() ) );	
	connect( ui.pushButton_last, SIGNAL(clicked()), this, SLOT( lastPage() ) );
	connect( ui.tabWidget_data, SIGNAL(currentChanged(int)), this, SLOT( setCurrentPage( int ) ));

	// buttons for sorting by attributes (exclude risk)
	connect( ui.tableWidget_original->horizontalHeader(), SIGNAL(sectionClicked ( int )), this, SLOT( sortByAttr( int ) ) );
	connect( ui.tableWidget_generalized->horizontalHeader(), SIGNAL(sectionClicked ( int )), this, SLOT( sortByAttr( int ) ) );
	//connect( ui.tableWidget_original->verticalHeader(), SIGNAL(sectionClicked ( int )), this, SLOT( emphasizeTuple( int ) ) );

	/* --- functions for left lower panel --- */

	// buttons for comparing contingency tables
	connect( ui.pushButton_compare, SIGNAL(clicked()), this, SLOT( createContingencyTable() ) );
	connect( ui.pushButton_compare, SIGNAL(clicked()), this, SLOT( calcChiSquareTest() ) );

	// buttons for comparing density graphs
	connect( ui.pushButton_compare_2, SIGNAL(clicked()), this, SLOT( plotJointDensity() ) );

	// buttons for comparing contingency tables
	ui.lineEdit_gof->setReadOnly( true );
	ui.lineEdit_gof->hide();
	ui.label_gof->hide();

	connect( ui.pushButton_compare_3, SIGNAL(clicked()), this, SLOT( plotMarginalDensityDifference() ) );
	connect( ui.tableWidget_contingency_generalized->horizontalScrollBar(), SIGNAL(sliderMoved( int )), this, SLOT( setOriginalCTHorizontal ( int ) ) );
	connect( ui.tableWidget_contingency_generalized->verticalScrollBar(), SIGNAL(sliderMoved( int )), this, SLOT( setOriginalCTVertical ( int ) ) );
	connect( ui.tableWidget_contingency_original->horizontalScrollBar(), SIGNAL(sliderMoved( int )), this, SLOT( setGeneralizedCTHorizontal ( int ) ) );
	connect( ui.tableWidget_contingency_original->verticalScrollBar(), SIGNAL(sliderMoved( int )), this, SLOT( setGeneralizedCTVertical ( int ) ) );

	/* --- signals from other windows to main window --- */

	connect( openFile, SIGNAL( signalBack(const QString&, const QString&) ), this, SLOT( readData(const QString&, const QString&) ) );
	connect( openHierarchy, SIGNAL( signalBack(const QTableWidget *) ), this, SLOT( readHierarchy(const QTableWidget *) ) );
	connect( openLog, SIGNAL( signalBack(const int) ), this, SLOT( revert(const int) ) );

	state = anony::INIT;
	setState();
}

void myMainWindow::setState()
{
	if ( state == anony::INIT )
	{
		// Set action
		ui.actionLoadHierarchy->setDisabled(true);
		ui.actionPublish->setDisabled(true);
		ui.actionMetaData->setDisabled(true);
		ui.actionGeneralization->setDisabled(true);

		// Initialize panel for load hierarchy file
		openLog->clear();

		// Set left-upper panel
		setCurrentPage( 0 );
		ui.pushButton_next->setDisabled(true);
		ui.pushButton_last->setDisabled(true);

		// Set right-upper panel
		ui.pushButton_delete->setDisabled(true);
		ui.pushButton_rescale->setDisabled(true);

		ui.doubleSpinBox_threshold->setDisabled(true);
		ui.doubleSpinBox_threshold->setValue( 0 );
		ui.horizontalSlider_threshold->setDisabled(true);
		ui.horizontalSlider_threshold->setValue( 0 );
		ui.doubleSpinBox_rescale->setDisabled(true);
		ui.doubleSpinBox_rescale->setValue( 0 );
		ui.horizontalSlider_rescale->setDisabled(true);
		ui.horizontalSlider_rescale->setValue( 0 );

		ui.lineEdit_number_deleted->clear();

		// Set left-lower panel
		ui.pushButton_compare->setDisabled(true);
		ui.pushButton_compare_2->setDisabled(true);
		ui.pushButton_compare_3->setDisabled(true);

		ui.tableWidget_contingency_generalized->clear();
		ui.tableWidget_contingency_generalized->setRowCount( 0 );
		ui.tableWidget_contingency_generalized->setColumnCount( 0 );
		ui.tableWidget_contingency_original->clear();
		ui.tableWidget_contingency_original->setRowCount( 0 );
		ui.tableWidget_contingency_original->setColumnCount( 0 );

		ui.comboBox_dimension->clear();
		ui.comboBox_x_dimension->clear();
		ui.comboBox_y_dimension->clear();
		ui.comboBox_x_dimension_2->clear();
		ui.comboBox_y_dimension_2->clear();

		QGraphicsScene* p_old_scene = ui.graphicsView_risk->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_generalized->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_original->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_original_2->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}
		
		p_old_scene = ui.graphicsView_generalized_2->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		// Set right-lower panel
		setCurrentVerifier( 0 );
		ui.horizontalSlider_c->setDisabled(true);
		ui.horizontalSlider_c->setValue( 0 );
		ui.horizontalSlider_t->setDisabled(true);
		ui.horizontalSlider_t->setValue( 0 );
		ui.horizontalSlider_l->setDisabled(true);
		ui.horizontalSlider_l->setValue( 0 );
		ui.doubleSpinBox_c->setDisabled(true);
		ui.doubleSpinBox_c->setValue( 0 );
		ui.doubleSpinBox_t->setDisabled(true);
		ui.doubleSpinBox_t->setValue( 0 );
		ui.spinBox_l->setDisabled(true);
		ui.spinBox_l->setValue( 0 );

		ui.pushButton_generalize->setDisabled(true);
		ui.pushButton_evaluaterisk->setDisabled(true);

		ui.spinBox_bk->setValue( 0 );
		ui.horizontalSlider_bk->setValue( 0 );

		ui.tableWidget_bk->clearContents();
		ui.tableWidget_bk->hide();
		ui.label_bk->hide();
	}
	else if ( state == anony::AFTER_DATA )
	{
		// Set action
		ui.actionLoadHierarchy->setEnabled(true);
		ui.actionPublish->setDisabled(true);
		ui.actionMetaData->setDisabled(true);
		ui.actionGeneralization->setDisabled(true);

		// Initialize panel for load hierarchy file
		openLog->clear();

		// Set left-upper panel
		setCurrentPage( 0 );
		ui.tableWidget_original->hideColumn( ui.tableWidget_original->columnCount() - 1 );
		ui.tableWidget_deleted->hideColumn( ui.tableWidget_original->columnCount() - 1 );

		// Set right-upper panel
		ui.pushButton_delete->setDisabled(true);
		ui.pushButton_rescale->setDisabled(true);

		ui.doubleSpinBox_threshold->setDisabled(true);
		ui.doubleSpinBox_threshold->setValue( 0 );
		ui.horizontalSlider_threshold->setDisabled(true);
		ui.horizontalSlider_threshold->setValue( 0 );
		ui.doubleSpinBox_rescale->setDisabled(true);
		ui.doubleSpinBox_rescale->setValue( 0 );
		ui.horizontalSlider_rescale->setDisabled(true);
		ui.horizontalSlider_rescale->setValue( 0 );

		ui.lineEdit_number_deleted->clear();

		// Set left-lower panel
		ui.pushButton_compare->setDisabled(true);
		ui.pushButton_compare_2->setDisabled(true);
		ui.pushButton_compare_3->setDisabled(true);

		ui.tableWidget_contingency_generalized->clear();
		ui.tableWidget_contingency_generalized->setRowCount( 0 );
		ui.tableWidget_contingency_generalized->setColumnCount( 0 );
		ui.tableWidget_contingency_original->clear();
		ui.tableWidget_contingency_original->setRowCount( 0 );
		ui.tableWidget_contingency_original->setColumnCount( 0 );

		QGraphicsScene* p_old_scene = ui.graphicsView_risk->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_generalized->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_original->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_original_2->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_generalized_2->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		// Set right-lower panel
		setCurrentVerifier( 0 );
		ui.horizontalSlider_c->setDisabled(true);
		ui.horizontalSlider_c->setValue( 0 );
		ui.horizontalSlider_l->setDisabled(true);
		ui.horizontalSlider_l->setValue( 0 );
		ui.doubleSpinBox_c->setDisabled(true);
		ui.doubleSpinBox_c->setValue( 0 );
		ui.spinBox_l->setDisabled(true);
		ui.spinBox_l->setValue( 0 );
		ui.horizontalSlider_t->setDisabled(true);
		ui.horizontalSlider_t->setValue( 0 );
		ui.doubleSpinBox_t->setDisabled(true);
		ui.doubleSpinBox_t->setValue( 0 );

		ui.pushButton_generalize->setDisabled(true);
		ui.pushButton_evaluaterisk->setDisabled(true);

		ui.spinBox_bk->setValue( 0 );
		ui.horizontalSlider_bk->setValue( 0 );

		ui.tableWidget_bk->clearContents();
		ui.tableWidget_bk->hide();
		ui.label_bk->hide();
	}
	else if ( state == anony::AFTER_HIERARCHY)
	{
		// Set action
		ui.actionLoadHierarchy->setEnabled(false);
		ui.actionMetaData->setDisabled(false);

		// Set left-upper panel
		setCurrentPage( 0 );

		// Set right-upper panel
		ui.pushButton_delete->setDisabled(true);
		ui.pushButton_rescale->setDisabled(true);

		ui.doubleSpinBox_threshold->setDisabled(true);
		ui.doubleSpinBox_threshold->setValue( 0 );
		ui.horizontalSlider_threshold->setDisabled(true);
		ui.horizontalSlider_threshold->setValue( 0 );
		ui.doubleSpinBox_rescale->setDisabled(true);
		ui.doubleSpinBox_rescale->setValue( 0 );
		ui.horizontalSlider_rescale->setDisabled(true);
		ui.horizontalSlider_rescale->setValue( 0 );

		ui.lineEdit_number_deleted->clear();

		// Set right-lower panel
		setCurrentVerifier( 0 );
		ui.horizontalSlider_c->setEnabled(true);
		ui.horizontalSlider_l->setEnabled(true);
		ui.doubleSpinBox_c->setEnabled(true);
		ui.spinBox_l->setEnabled(true);
		ui.horizontalSlider_t->setEnabled(true);
		ui.doubleSpinBox_t->setEnabled(true);
		ui.horizontalSlider_c->setValue( 0 );
		ui.horizontalSlider_l->setValue( 0 );
		ui.doubleSpinBox_c->setValue( 0 );
		ui.spinBox_l->setValue( 0 );
		ui.horizontalSlider_t->setValue( 0 );
		ui.doubleSpinBox_t->setValue( 0 );

		ui.pushButton_generalize->setEnabled(true);
		ui.pushButton_evaluaterisk->setDisabled(true);

		ui.spinBox_bk->setValue( 0 );
		ui.horizontalSlider_bk->setValue( 0 );

		ui.tableWidget_bk->hide();

		// Set left-lower panel
		ui.pushButton_compare->setDisabled(true);
		ui.pushButton_compare_2->setDisabled(true);
		ui.pushButton_compare_3->setDisabled(true);

		ui.tableWidget_contingency_generalized->clear();
		ui.tableWidget_contingency_generalized->setRowCount( 0 );
		ui.tableWidget_contingency_generalized->setColumnCount( 0 );
		ui.tableWidget_contingency_original->clear();
		ui.tableWidget_contingency_original->setRowCount( 0 );
		ui.tableWidget_contingency_original->setColumnCount( 0 );

		QGraphicsScene* p_old_scene = ui.graphicsView_risk->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_generalized->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_original->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_original_2->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_generalized_2->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}
	}
	else if ( state == anony::AFTER_GENERALIZATION )
	{
		// Set action
		ui.actionPublish->setDisabled(false);
		ui.actionGeneralization->setDisabled(false);

		// Set left-upper panel
		setCurrentPage( 1 );

		// Set right-upper panel
		ui.pushButton_delete->setDisabled(true);
		ui.pushButton_rescale->setDisabled(true);

		ui.doubleSpinBox_threshold->setDisabled(true);
		ui.doubleSpinBox_threshold->setValue( 0 );
		ui.horizontalSlider_threshold->setDisabled(true);
		ui.horizontalSlider_threshold->setValue( 0 );

		ui.doubleSpinBox_rescale->setDisabled(true);
		ui.doubleSpinBox_rescale->setValue( 0 );
		ui.horizontalSlider_rescale->setDisabled(true);
		ui.horizontalSlider_rescale->setValue( 0 );

		ui.lineEdit_number_deleted->clear();

		// Set right-lower panel
		ui.pushButton_evaluaterisk->setDisabled(false);
		ui.tableWidget_bk->show();

		// Set left-lower panel
		ui.pushButton_compare->setDisabled(false);
		ui.pushButton_compare_2->setDisabled(false);
		ui.pushButton_compare_3->setDisabled(false);

		ui.tableWidget_contingency_generalized->clear();
		ui.tableWidget_contingency_generalized->setRowCount( 0 );
		ui.tableWidget_contingency_generalized->setColumnCount( 0 );
		ui.tableWidget_contingency_original->clear();
		ui.tableWidget_contingency_original->setRowCount( 0 );
		ui.tableWidget_contingency_original->setColumnCount( 0 );

		QGraphicsScene* p_old_scene = ui.graphicsView_risk->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_generalized->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_original->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
			plotJointDensity();
		}

		p_old_scene = ui.graphicsView_original_2->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
		}

		p_old_scene = ui.graphicsView_generalized_2->scene();
		if (p_old_scene != NULL)
		{
			delete p_old_scene;
			plotMarginalDensityDifference();
		}
	}
	else if ( state == anony::AFTER_RISK )
	{
		// Set left-upper panel
		ui.tableWidget_original->showColumn( ui.tableWidget_original->columnCount() - 1 );
		ui.tableWidget_deleted->showColumn( ui.tableWidget_deleted->columnCount() - 1 );
		
		// Set right-upper panel
		ui.pushButton_delete->setDisabled(false);
		ui.pushButton_rescale->setDisabled(false);

		ui.doubleSpinBox_threshold->setDisabled(false);
		ui.horizontalSlider_threshold->setDisabled(false);
		ui.doubleSpinBox_rescale->setDisabled(false);
		ui.horizontalSlider_rescale->setDisabled(false);

		ui.lineEdit_number_deleted->clear();
	}
	else if ( state == anony::AFTER_DELETE )
	{
		// Set left-upper panel
		setCurrentPage( 2 );
	}
}

void myMainWindow::setOriginalCTHorizontal(int i)
{
	ui.tableWidget_contingency_original->horizontalScrollBar()->setSliderPosition ( i );
}

void myMainWindow::setOriginalCTVertical(int i)
{
	ui.tableWidget_contingency_original->verticalScrollBar()->setSliderPosition ( i );
}

void myMainWindow::setGeneralizedCTHorizontal(int i)
{
	ui.tableWidget_contingency_generalized->horizontalScrollBar()->setSliderPosition ( i );
}

void myMainWindow::setGeneralizedCTVertical(int i)
{
	ui.tableWidget_contingency_generalized->verticalScrollBar()->setSliderPosition ( i );
}

void myMainWindow::numTuple(double threshold)
{
	if (threshold == 0)
	{
		threshold = ui.doubleSpinBox_threshold->value();
	}

	int count = 0;
	vector<float> vecRisk = generalizor.m_vecRisk;
	for(int i = 0; i < vecRisk.size(); i++){

		if ( vecRisk[i] >= threshold)
		{
			count++;
		}
	}

	ui.lineEdit_number_deleted->setText(QString::number( count ));
}

myMainWindow::~myMainWindow()
{
	delete openFile;
	delete openHierarchy;
	delete openLog;
	delete showMeta;
	delete showGen;

	QGraphicsScene* p_old_scene = ui.graphicsView_risk->scene();
	if (p_old_scene != NULL)
	{
		delete p_old_scene;
	}	

	vec_del.clear();
	vec_hrch.clear();
	vec_log.clear();
	vec_qi.clear();
	vec_order.clear();
}

void myMainWindow::setInt2Double_c(int value)
{
	double valueDouble = (double)value / 100;
	ui.doubleSpinBox_c->setValue( valueDouble );
}

void myMainWindow::setDouble2Int_c(double value)
{
	int valueInt = value * 100;
	ui.horizontalSlider_c->setValue( valueInt );
}

void myMainWindow::setInt2Double_t(int value)
{
	double valueDouble = (double)value / 1000;
	ui.doubleSpinBox_t->setValue( valueDouble );
}

void myMainWindow::setDouble2Int_t(double value)
{
	int valueInt = value * 1000;
	ui.horizontalSlider_t->setValue( valueInt );
}

void myMainWindow::setInt2Double_th(int value)
{
	double valueDouble = (double)value / 100;
	ui.doubleSpinBox_threshold->setValue( valueDouble );
}

void myMainWindow::setDouble2Int_th(double value)
{
	int valueInt = value * 100;
	ui.horizontalSlider_threshold->setValue( valueInt );
}

void myMainWindow::setInt2Double_r(int value)
{
	double valueDouble = (double)value / 100;
	ui.doubleSpinBox_rescale->setValue( valueDouble );
}

void myMainWindow::setDouble2Int_r(double value)
{
	int valueInt = value * 100;
	ui.horizontalSlider_rescale->setValue( valueInt );
}

void myMainWindow::changeThreshold(double threshold)
{
	thresHold->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + ( threshold - minRisk ) * BIN_NUM * BIN_WIDTH / ( maxRisk - minRisk ), TOP_MARGIN, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + ( threshold - minRisk ) * BIN_NUM * BIN_WIDTH /( maxRisk - minRisk ), TOP_MARGIN + BIN_NUM * BIN_HEIGHT );
}

void myMainWindow::on_actionLoadData_triggered()
{
	openFile->show();
	openFile->activateWindow();
}

void myMainWindow::on_actionLoadHierarchy_triggered()
{
	openHierarchy->show();
	openHierarchy->activateWindow();
}

void myMainWindow::on_actionShowLog_triggered()
{
	openLog->show();
	openLog->activateWindow();
}

void myMainWindow::on_actionMetaData_triggered()
{
	showMeta->show();
	showMeta->activateWindow();
}

void myMainWindow::on_actionGeneralization_triggered()
{
	showGen->show();
	showGen->activateWindow();
}

void myMainWindow::on_actionPublish_triggered()
{
	QString path;

	path = QFileDialog::getSaveFileName(
		this,
		"Save As",
		QString::null,
		QString::null);
	
	ofstream publish_file( path.toLatin1().data() );
	for ( int indexRow = 0; indexRow < data.GetTupleNum(); indexRow++ )
	{
		if ( generalizor.m_vecStatus[indexRow] == anony::NORMAL )
		{
			for ( int indexColumn = 0; indexColumn < data.GetVecMeta().size(); indexColumn++ )
			{
				publish_file << generalizor.GetValueQstr(indexRow, indexColumn).toLatin1().data() << "\t";
			}
			publish_file << "\n";
		}
	}

	publish_file.close();
}

void myMainWindow::on_actionAbout_triggered()
{
	aboutCAT->show();
	aboutCAT->activateWindow();
}


void myMainWindow::on_actionExit_triggered()
{
	delete this;
}


void myMainWindow::readData(const QString& microPath, const QString& metaPath)
{
	/* -------------- read in data, initialize vector order -------------- */
	try
	{
		data.ReadFiles( microPath.toLatin1().data(), metaPath.toLatin1().data() );
	}
	catch( AnonyException e )
	{
		QMessageBox::information(this, "Message", e.m_strMsg.c_str());

		state = anony::INIT;
		setState();

		return;
	}
	
	const vector<Metadata> &vectorMeta = data.GetVecMeta();
	for ( int order = 0; order < data.GetTupleNum(); order++ )
	{
		vec_order.push_back( order );
	}

	/* -------------- set page count -------------- */
	originalCurrentPage = 1;
	maxPage = data.GetVecTuple().size() / vectorMeta.size() / PAGE_SIZE;
	maxPage += (data.GetVecTuple().size() / vectorMeta.size() % PAGE_SIZE == 0) ? 0 : 1;

	if (maxPage > 1)
	{
		ui.pushButton_next->setDisabled(false);
	}

	/* -------------- table for rendering original, generalized and deleted data --------------*/
	int upperBound = ( vec_order.size() > PAGE_SIZE ? PAGE_SIZE : data.GetTupleNum());
	ui.tableWidget_original->clearContents();
	ui.tableWidget_original->setColumnCount( vectorMeta.size() + 1 );
	ui.tableWidget_original->setRowCount( upperBound );
	ui.tableWidget_original->setEditTriggers( QAbstractItemView::NoEditTriggers );

	// read in attribute names
	int indexColumn;
	QStringList temp_list;
	for(indexColumn = 0; indexColumn < ui.tableWidget_original->columnCount() - 1; indexColumn++)
	{
		QString qstr(vectorMeta[indexColumn].m_strName.c_str());
		temp_list << qstr;
	}

	ui.tableWidget_generalized->clearContents();
	ui.tableWidget_generalized->setColumnCount( vectorMeta.size() );
	ui.tableWidget_generalized->setRowCount( 0 );

	// generalized table does not have risk attribute
	ui.tableWidget_generalized->setHorizontalHeaderLabels( temp_list );
	ui.tableWidget_deleted->clearContents();
	ui.tableWidget_deleted->setColumnCount( vectorMeta.size() + 1 );
	ui.tableWidget_deleted->setRowCount( 0 );

	//add attributes to combox of utility graphs for x-dimension and y-dimension, and sorting attributes
	ui.comboBox_dimension->clear();
	ui.comboBox_x_dimension->clear();
	ui.comboBox_y_dimension->clear();
	ui.comboBox_x_dimension_2->clear();
	ui.comboBox_y_dimension_2->clear();

	ui.comboBox_dimension->addItems( temp_list );
	ui.comboBox_x_dimension->addItems( temp_list);
	ui.comboBox_y_dimension->addItems( temp_list);
	ui.comboBox_x_dimension_2->addItems( temp_list);
	ui.comboBox_y_dimension_2->addItems( temp_list);

	// original and deleted tables do have risk attribute
	temp_list << " Risk ";
	ui.tableWidget_original->setHorizontalHeaderLabels( temp_list );
	ui.tableWidget_deleted->setHorizontalHeaderLabels( temp_list );

	/* -------------- titles of the original and deleted tables colored "red" for "risk" -------------- */
	QBrush red_brush(Qt::red);
	QBrush black_brush(Qt::black);
	QFont font_bold;
	font_bold.setBold(true);

	for(indexColumn = 0; indexColumn < ui.tableWidget_original->columnCount(); indexColumn++)
	{
		ui.tableWidget_original->horizontalHeaderItem (indexColumn)->setFont(font_bold);
		ui.tableWidget_deleted->horizontalHeaderItem (indexColumn)->setFont(font_bold);
		ui.tableWidget_original->horizontalHeaderItem (indexColumn)->setForeground(black_brush);
		ui.tableWidget_deleted->horizontalHeaderItem (indexColumn)->setForeground(black_brush);

		if(indexColumn >= ui.tableWidget_generalized->columnCount())
		{
			continue;
		}
		ui.tableWidget_generalized->horizontalHeaderItem (indexColumn)->setFont(font_bold);
		ui.tableWidget_generalized->horizontalHeaderItem (indexColumn)->setForeground(black_brush);
	}
	ui.tableWidget_original->horizontalHeaderItem (ui.tableWidget_original->columnCount() - 1)->setForeground(red_brush);
	ui.tableWidget_deleted->horizontalHeaderItem (ui.tableWidget_original->columnCount() - 1)->setForeground(red_brush);

	/* -------------- insert values for the original table -------------- */
	int indexRow;
	QTableWidgetItem* p_item;
	for(indexRow = 0; indexRow < upperBound; indexRow++){

		for (indexColumn = 0; indexColumn < ui.tableWidget_original->columnCount() - 1; indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( data.GetValueQstr( vec_order[indexRow], indexColumn ) );
			ui.tableWidget_original->setItem( indexRow, indexColumn, p_item );
		}

		// set risk to N/A originally
		p_item = new QTableWidgetItem;
		p_item->setText( "N/A" );
		ui.tableWidget_original->setItem( indexRow, ui.tableWidget_original->columnCount() - 1, p_item );
	}
	ui.tableWidget_original->resizeColumnsToContents();

	openHierarchy->loadMeta(vectorMeta);
	openHierarchy->ui.lineEdit_hierarchy->clear();

	state = anony::AFTER_DATA;
	setState();
}

void myMainWindow::readHierarchy(const QTableWidget *qi)
{
	const int rowCount = qi->rowCount();
	const vector<Metadata> &vectorMeta = data.GetVecMeta();

	vec_qi.clear();
	vec_hrch.clear();

	/* -------------- read in "sa" and "qi" attributes -------------- */
	int rowIndex;
	for(rowIndex = 0; rowIndex < rowCount; rowIndex++){

		if(((QCheckBox *) qi->cellWidget(rowIndex, 2))->isChecked())
		{
			vec_qi.push_back( rowIndex );
			vec_hrch.push_back( ((myLineEdit *) qi->cellWidget(rowIndex, 3))->text().toLatin1().data() );
		}

		if(((QCheckBox *) qi->cellWidget(rowIndex, 1))->isChecked())
		{
			sensAttr = rowIndex;
		}
	}

	/* -------------- initialize table for setting background knowledge -------------- */
	try
	{
		generalizor.Init( &data, vec_hrch, vec_qi, sensAttr );
	}
	catch( AnonyException e )
	{
		QMessageBox::information(this, "Message", e.m_strMsg.c_str());

		state = anony::AFTER_DATA;
		setState();

		return;
	}
	
	QStringList temp_list;
	temp_list << QString("  ") << QString("Background Knowledge of QI Attributes");
	ui.tableWidget_bk->clearContents();
	ui.tableWidget_bk->setColumnCount( 2 );
	ui.tableWidget_bk->setRowCount( vec_hrch.size() );
	ui.tableWidget_bk->setHorizontalHeaderLabels( temp_list );
	ui.label_bk->setText(QString("Pieces of background knowledge of ") + vectorMeta[sensAttr].m_strName.c_str());

	QBrush blue_brush(Qt::blue);
	QBrush red_brush(Qt::red);
	QBrush magenta_brush(Qt::magenta);

	int indexRow;
	QTableWidgetItem* p_item;
	QCheckBox *cbox;
	for(indexRow = 0; indexRow < ui.tableWidget_bk->rowCount(); indexRow++){

		p_item = new QTableWidgetItem;
		p_item->setText( QString( vectorMeta[vec_qi[indexRow]].m_strName.c_str() ) );
		ui.tableWidget_bk->setItem( indexRow, 1, p_item );

		cbox = new QCheckBox();
		ui.tableWidget_bk->setCellWidget( indexRow, 0, cbox );
		
		//titles of the original tables colored "blue" for "qi"
		ui.tableWidget_original->horizontalHeaderItem (vec_qi[indexRow])->setForeground(blue_brush);
		ui.tableWidget_generalized->horizontalHeaderItem (vec_qi[indexRow])->setForeground(blue_brush);
		ui.tableWidget_deleted->horizontalHeaderItem (vec_qi[indexRow])->setForeground(blue_brush);

	}
	ui.tableWidget_bk->resizeColumnsToContents();

	//hide vertical headers
	ui.tableWidget_bk->verticalHeader()->hide();

	//titles of the original tables colored "red" for "sa"
	ui.tableWidget_original->horizontalHeaderItem (sensAttr)->setForeground(magenta_brush);
	ui.tableWidget_deleted->horizontalHeaderItem (sensAttr)->setForeground(magenta_brush);

	showMeta->loadMeta( generalizor );

	state = anony::AFTER_HIERARCHY;
	setState();
}

void myMainWindow::generalize( bool redo )
{	
	generalizor.m_vecRisk.clear();

	/* ---- Using L-Diversity ---- */
	if ( ui.tabWidget_verifier->currentWidget() == ui.tab_diversity )
	{
		int value_l;
		double value_c;

		value_l = ui.spinBox_l->value();
		value_c = ui.doubleSpinBox_c->value();

		if ( !redo )
		{
			myLog newLog;

			newLog.para_l = value_l;
			newLog.para_c = value_c;
			newLog.para_vec_del = vec_del;
			newLog.type_verifier = anony::DIVERSITY;
			newLog.timestamp = QTime::currentTime();
			newLog.type_log = anony::GENERALIZATION;

			for ( int index = openLog->current_state + 1; index < vec_log.size(); index++ )
			{
				vec_log.pop_back();
			}
			vec_log.push_back( newLog );
			openLog->UpdateLog( newLog );
		}

		/* -------------- execute generalization -------------- */

		L_Diversity diversity( value_l, value_c );

		try
		{
			diversity.CheckBeforeAnonymize( generalizor );
		}
		catch( AnonyException e )
		{
			QMessageBox::information(this, "Message", e.m_strMsg.c_str());

			return;
		}

		this->setCursor( Qt::WaitCursor );
		try
		{
			generalizor.Anonymize(diversity);
		}
		catch( AnonyException e )
		{
			QMessageBox::information(this, "Message", e.m_strMsg.c_str());

			state = anony::AFTER_HIERARCHY;
			setState();

			return;
		}
		this->setCursor( Qt::ArrowCursor );
	}

	/* ---- Using T-Closeness ---- */
	else if ( ui.tabWidget_verifier->currentWidget() == ui.tab_closeness )
	{
		double value_t;
		value_t = ui.doubleSpinBox_t->value();

		if ( !redo )
		{
			myLog newLog;

			newLog.para_t = value_t;
			newLog.para_vec_del = vec_del;
			newLog.type_verifier = anony::CLOSENESS;
			newLog.timestamp = QTime::currentTime();
			newLog.type_log = anony::GENERALIZATION;

			for ( int index = openLog->current_state + 1; index < vec_log.size(); index++ )
			{
				vec_log.pop_back();
			}
			vec_log.push_back( newLog );
			openLog->UpdateLog( newLog );
		}

		/* -------------- execute generalization -------------- */

		T_Closeness closeness( value_t, generalizor );

		try
		{
			closeness.CheckBeforeAnonymize( generalizor );
		}
		catch( AnonyException e )
		{
			QMessageBox::information(this, "Message", e.m_strMsg.c_str());

			return;
		}

		this->setCursor( Qt::WaitCursor );
		try
		{
			generalizor.Anonymize(closeness);
		}
		catch( AnonyException e )
		{
			QMessageBox::information(this, "Message", e.m_strMsg.c_str());

			state = anony::AFTER_HIERARCHY;
			setState();

			return;
		}
		this->setCursor( Qt::ArrowCursor );
	}

	/* -------------- render table for generalized data -------------- */
	int upperBound = ( vec_order.size() > PAGE_SIZE ? PAGE_SIZE : data.GetTupleNum());
	generalizedCurrentPage = 1;
	ui.tableWidget_generalized->setRowCount( upperBound );
	ui.tableWidget_generalized->setEditTriggers( QAbstractItemView::NoEditTriggers );

	QBrush blue_brush(Qt::blue);
	QBrush red_brush(Qt::red);
	QBrush magenta_brush(Qt::magenta);
	QFont font_bold;
	font_bold.setBold(true);

	int indexColumn;
	for(indexColumn = 0; indexColumn < ui.tableWidget_generalized->columnCount(); indexColumn++)
	{
		ui.tableWidget_generalized->horizontalHeaderItem (indexColumn)->setFont(font_bold);
	}

	//titles of the generalized tables colored "blue" for "qi"
	for(int indexRow = 0; indexRow < ui.tableWidget_bk->rowCount(); indexRow++)
	{
		ui.tableWidget_generalized->horizontalHeaderItem (vec_qi[indexRow])->setForeground(blue_brush);
	}

	//titles of the generalized tables colored "red" for "sa"
	ui.tableWidget_generalized->horizontalHeaderItem (sensAttr)->setForeground(magenta_brush);

	int indexRow;
	QTableWidgetItem* p_item;
	for(indexRow = 0; indexRow < upperBound; indexRow++)
	{
		for(indexColumn = 0; indexColumn < ui.tableWidget_generalized->columnCount(); indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( generalizor.GetValueQstr( vec_order[indexRow], indexColumn ) );
			ui.tableWidget_generalized->setItem( indexRow, indexColumn, p_item );
		}
	}
	ui.tableWidget_generalized->resizeColumnsToContents();

	/* -------------- resume table for original data -------------- */
	for(indexRow = 0; indexRow < upperBound; indexRow++){

		p_item = new QTableWidgetItem;
		p_item->setText( "N/A" );
		ui.tableWidget_original->setItem( indexRow, ui.tableWidget_original->columnCount() - 1, p_item );
	}
	ui.tableWidget_original->resizeColumnsToContents();

	showGen->loadGen( generalizor );

	state = anony::AFTER_GENERALIZATION;
	setState();

	/* -------------- update risk and utility window right after generalization -------------- */
	if ( !redo )
	{
		for(int rowIndex = 0; rowIndex < ui.tableWidget_bk->rowCount(); rowIndex++)
		{
			if( ((QCheckBox *) ui.tableWidget_bk->cellWidget( rowIndex, 0 ))->checkState() == Qt::Checked )
			{
				evaluateRisk( false, true );
				break;				
			}
		}
	}
}

void myMainWindow::evaluateRisk( bool redo, bool needsUpdate )
{
	long nBckKnw = (long) ui.spinBox_bk->value();
	vector<long> rVecAttr;
	vector<long> rVecCnt;
	int rowIndex;
	float maxRisk;

	for(rowIndex = 0; rowIndex < ui.tableWidget_bk->rowCount(); rowIndex++)
	{
		if( ((QCheckBox *) ui.tableWidget_bk->cellWidget( rowIndex, 0 ))->checkState() == Qt::Checked ){

			rVecAttr.push_back(rowIndex);
		}
	}

	/* ---- Using L-Diversity ---- */
	if ( ui.tabWidget_verifier->currentWidget() == ui.tab_diversity )
	{
		if ( !redo )
		{
			myLog newLog;
			newLog.para_vec_bg_qi = rVecAttr;
			newLog.para_piece_sa = (long) ui.spinBox_bk->value();
			newLog.timestamp = QTime::currentTime();
			newLog.type_log = anony::EVALUATE;
			newLog.type_verifier = anony::DIVERSITY;
			vec_log.push_back( newLog );
			openLog->UpdateLog( newLog );
		}

		// execute risk evaluation and plot the histogram
		L_Diversity diversity;
		diversity.m_nBckKnw = nBckKnw;

		try
		{
			diversity.CheckBeforeEvalRisk( generalizor );
		}
		catch( AnonyException e )
		{
			QMessageBox::information(this, "Message", e.m_strMsg.c_str());

			return;
		}

		try
		{
			generalizor.EvaluateRisk( rVecAttr, diversity, BIN_NUM, rVecCnt, maxRisk, 0, needsUpdate );
		}
		catch( AnonyException e )
		{
			QMessageBox::information(this, "Message", e.m_strMsg.c_str());

			state = anony::AFTER_GENERALIZATION;
			setState();

			return;
		}
	}

	/* ---- Using T-Closeness ---- */
	else if ( ui.tabWidget_verifier->currentWidget() == ui.tab_closeness )
	{
		if ( !redo )
		{
			myLog newLog;
			newLog.para_vec_bg_qi = rVecAttr;
			newLog.timestamp = QTime::currentTime();
			newLog.type_log = anony::EVALUATE;
			newLog.type_verifier = anony::CLOSENESS;
			vec_log.push_back( newLog );
			openLog->UpdateLog( newLog );
		}

		// execute risk evaluation and plot the histogram
		T_Closeness closeness( 0.0f, generalizor);
		
		try
		{
			closeness.CheckBeforeEvalRisk( generalizor );
		}
		catch( AnonyException e )
		{
			QMessageBox::information(this, "Message", e.m_strMsg.c_str());

			return;
		}

		try
		{
			generalizor.EvaluateRisk( rVecAttr, closeness, BIN_NUM, rVecCnt, maxRisk, 0, needsUpdate );
		}
		catch( AnonyException e )
		{
			QMessageBox::information(this, "Message", e.m_strMsg.c_str());

			state = anony::AFTER_GENERALIZATION;
			setState();

			return;
		}

	}

	ui.doubleSpinBox_threshold->setRange( 0, maxRisk );
	ui.horizontalSlider_threshold->setRange( 0, maxRisk * 100 );

	QGraphicsScene* p_old_scene = ui.graphicsView_risk->scene();
	QGraphicsScene* p_new_scene = new QGraphicsScene;
	genHisto( rVecCnt, maxRisk, 0, *p_new_scene );
	ui.graphicsView_risk->setScene( p_new_scene );
	QRectF boundRec = p_new_scene->sceneRect();
	ui.graphicsView_risk->fitInView(boundRec);
	if (p_old_scene != NULL)
	{
		delete p_old_scene;
	}	

	/* -------------- add disclosure risk -------------- */
	int upperBound = ( vec_order.size() > PAGE_SIZE ? PAGE_SIZE : vec_order.size());
	for (rowIndex = 0; rowIndex < upperBound; rowIndex++)
	{
		QTableWidgetItem *p_item = ui.tableWidget_original->item(rowIndex, ui.tableWidget_original->columnCount() - 1);
		p_item->setText( QString::number( (int) (generalizor.m_vecRisk[vec_order[(originalCurrentPage - 1) * PAGE_SIZE + rowIndex] ] * 100)) + "%" );
	}

	state = anony::AFTER_RISK;
	setState();
}

void myMainWindow::rescaleRisk()
{
	float minRisk = (float) ui.doubleSpinBox_rescale->value();
	vector<long> rVecAttr;
	vector<long> rVecCnt;
	float maxRisk;

	L_Diversity dummy;
	try
	{
		generalizor.EvaluateRisk( rVecAttr, dummy, BIN_NUM, rVecCnt, maxRisk, minRisk, false );
	}
	catch( AnonyException e )
	{
		QMessageBox::information(this, "Message", e.m_strMsg.c_str());

		state = anony::AFTER_GENERALIZATION;
		setState();

		return;
	}

	ui.doubleSpinBox_threshold->setRange( minRisk, maxRisk );
	ui.horizontalSlider_threshold->setRange( minRisk * 100, maxRisk * 100 );

	QGraphicsScene* p_old_scene = ui.graphicsView_risk->scene();
	QGraphicsScene* p_new_scene = new QGraphicsScene;
	genHisto( rVecCnt, maxRisk, minRisk, *p_new_scene );
	ui.graphicsView_risk->setScene( p_new_scene );
	QRectF boundRec = p_new_scene->sceneRect();
	ui.graphicsView_risk->fitInView(boundRec);
	if (p_old_scene != NULL)
	{
		delete p_old_scene;
	}	
}


void myMainWindow::deleteTuple( bool redo )
{
	vector<float> temp_risk = generalizor.m_vecRisk;
	long tuplesDeleted = ui.lineEdit_number_deleted->text().toLong();
	sort( temp_risk.begin(), temp_risk.end(), greater<float>() );
	while( temp_risk[tuplesDeleted] == temp_risk[tuplesDeleted - 1] )
	{
		tuplesDeleted++;
	}
	temp_risk.clear();

	if ( !redo )
	{
		QMessageBox::StandardButton return_key = QMessageBox::question(this, "Confirmation", QString::number(tuplesDeleted) + " tuples will be actually deleted, continue?", QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);

		if ( return_key == QMessageBox::No)
		{
			return;
		}
	}

	if( !redo ){

		myLog newLog;
		newLog.para_num_del = tuplesDeleted;
		newLog.timestamp = QTime::currentTime();
		newLog.type_log = anony::DELETE;
		vec_log.push_back( newLog );
		openLog->UpdateLog( newLog );
	}

	generalizor.DelTopRisk( tuplesDeleted );
	generalizor.Clean( vec_order );

	vec_del.clear();
	for ( int indexRow = 0; indexRow < generalizor.m_vecStatus.size(); indexRow++ )
	{
		if ( generalizor.m_vecStatus[ indexRow ] == anony::DELETED )
		{
			vec_del.push_back( indexRow );
		}
	}
	updateDeleted();

	/* -------------- update risk histogram and utility graphs -------------- */
	evaluateRisk( true, false );

	if ( ui.graphicsView_original->scene() != NULL && ui.graphicsView_generalized->scene() != NULL )
	{
		plotJointDensity();
	}
	if ( ui.graphicsView_original_2->scene() != NULL && ui.graphicsView_generalized_2->scene() != NULL )
	{
		plotMarginalDensityDifference();
	}
	if ( ui.tableWidget_contingency_generalized->rowCount() != 0 && ui.tableWidget_contingency_original->rowCount() != 0 )
	{
		createContingencyTable();
	}

	state = anony::AFTER_DELETE;
	setState();
}


void myMainWindow::updateDeleted()
{
	/* -------------- resume to the first page for both original and generalized tables -------------- */
	int indexRow;
	int upperBound = ( vec_order.size() > PAGE_SIZE ? PAGE_SIZE : vec_order.size());
	ui.tableWidget_original->setRowCount( upperBound );

	QTableWidgetItem* p_item;
	for(indexRow = 0; indexRow < upperBound; indexRow++){

		for (int indexColumn = 0; indexColumn < ui.tableWidget_original->columnCount() - 1; indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( data.GetValueQstr( vec_order[indexRow], indexColumn ) );
			ui.tableWidget_original->setItem( indexRow, indexColumn, p_item );
		}

		// add disclosure risk
		if (generalizor.m_vecRisk.empty())
		{
			p_item = new QTableWidgetItem;
			p_item->setText( "N/A" );
			ui.tableWidget_original->setItem( indexRow, ui.tableWidget_original->columnCount() - 1, p_item );
		}
		else
		{
			p_item = new QTableWidgetItem;
			p_item->setText( QString::number( (int) (generalizor.m_vecRisk[vec_order[indexRow] ] * 100)) + "%" );
			ui.tableWidget_original->setItem( indexRow, ui.tableWidget_original->columnCount() - 1, p_item );
		}
	}
	ui.tableWidget_original->resizeColumnsToContents();
	originalCurrentPage = 1;

	ui.tableWidget_generalized->setRowCount( upperBound );
	for(indexRow = 0; indexRow < upperBound; indexRow++)
	{
		for(int indexColumn = 0; indexColumn < ui.tableWidget_generalized->columnCount(); indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( generalizor.GetValueQstr( vec_order[indexRow], indexColumn ) );
			ui.tableWidget_generalized->setItem( indexRow, indexColumn, p_item );
		}
	}
	ui.tableWidget_generalized->resizeColumnsToContents();
	generalizedCurrentPage = 1;

	/* -------------- insert values in to deleted table -------------- */
	maxDeletePage = vec_del.size() / PAGE_SIZE;
	maxDeletePage += (vec_del.size() % PAGE_SIZE == 0) ? 0 : 1;
	ui.tableWidget_deleted->clearContents();
	ui.tableWidget_deleted->setRowCount( vec_del.size() >= PAGE_SIZE ? PAGE_SIZE : vec_del.size() );
	ui.tableWidget_deleted->setColumnCount( data.GetVecMeta().size() + 1 );
	ui.tableWidget_deleted->setEditTriggers( QAbstractItemView::NoEditTriggers );
	deletedCurrentPage = 1;

	for ( int indexRow = 0; indexRow < ui.tableWidget_deleted->rowCount(); indexRow++ )
	{
		int dataIndexRow = vec_del[ indexRow ];
		for ( int indexColumn = 0; indexColumn < ui.tableWidget_deleted->columnCount() - 1; indexColumn++ )
		{
			p_item = new QTableWidgetItem;
			p_item->setText( data.GetValueQstr( dataIndexRow, indexColumn ) );
			ui.tableWidget_deleted->setItem( indexRow, indexColumn, p_item);
		}

		// add disclosure risk
		if (generalizor.m_vecRisk.empty())
		{
			p_item = new QTableWidgetItem;
			p_item->setText( "N/A" );
			ui.tableWidget_deleted->setItem( indexRow, ui.tableWidget_deleted->columnCount() - 1, p_item );
		}
		else 
		{
			p_item = new QTableWidgetItem;
			p_item->setText( QString::number( (int) (generalizor.m_vecRisk[ dataIndexRow ] * 100)) + "%" );
			ui.tableWidget_deleted->setItem( indexRow, ui.tableWidget_deleted->columnCount() - 1, p_item );
		}
	}
	ui.tableWidget_deleted->resizeColumnsToContents();
}

void myMainWindow::sortByAttr(int attrIndex)
{
	if ( attrIndex != sortIndex )
	{
		ascend = false;
	}
	else
	{
		ascend = !ascend;
	}
	
	if ( attrIndex == ui.tableWidget_original->columnCount() - 1 )
	{
		if ( generalizor.m_vecRisk.empty() )
		{
			return;
		}
		else
		{
			generalizor.SortByRisk(vec_order, ascend);
		}
	}
	else{

		data.SortByAttr(attrIndex, vec_order, ascend);
	}

	sortIndex = attrIndex;
	if ( ascend )
	{
		ui.tableWidget_original->horizontalHeader()->setSortIndicator( sortIndex, Qt::AscendingOrder );
		ui.tableWidget_generalized->horizontalHeader()->setSortIndicator( sortIndex, Qt::AscendingOrder );
	}
	else
	{
		ui.tableWidget_original->horizontalHeader()->setSortIndicator( sortIndex, Qt::DescendingOrder );
		ui.tableWidget_generalized->horizontalHeader()->setSortIndicator( sortIndex, Qt::DescendingOrder );
	}
	ui.tableWidget_original->horizontalHeader()->setSortIndicatorShown ( true );
	ui.tableWidget_generalized->horizontalHeader()->setSortIndicatorShown ( true );

	ui.tableWidget_original->resizeColumnsToContents();
	if ( generalizor.m_bGenExists )
	{
		ui.tableWidget_generalized->resizeColumnsToContents();
	}
	generalizor.Clean( vec_order );

	/* -------------- update original and generalized table -------------- */
	int indexRow, indexColumn;
	int upperBound = ( originalCurrentPage == maxPage ? data.GetTupleNum() % PAGE_SIZE : PAGE_SIZE);
	QTableWidgetItem *p_item;
	for (indexRow = 0; indexRow < upperBound; indexRow++)
	{
		for (indexColumn = 0; indexColumn < ui.tableWidget_original->columnCount() - 1; indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( data.GetValueQstr( vec_order[(originalCurrentPage - 1) * PAGE_SIZE + indexRow], indexColumn ) );
			ui.tableWidget_original->setItem( indexRow, indexColumn, p_item );
		}

		if (generalizor.m_vecRisk.empty())
		{
			p_item = new QTableWidgetItem;
			p_item->setText( "N/A" );
			ui.tableWidget_original->setItem( indexRow, ui.tableWidget_original->columnCount() - 1, p_item );
		}
		else 
		{
			p_item = new QTableWidgetItem;
			p_item->setText( QString::number( (int) (generalizor.m_vecRisk[vec_order[(originalCurrentPage - 1) * PAGE_SIZE + indexRow] ] * 100)) + "%"  );
			ui.tableWidget_original->setItem( indexRow, ui.tableWidget_original->columnCount() - 1, p_item );
		}
	}

	if ( generalizor.m_bGenExists == true )
	{
		upperBound = ( generalizedCurrentPage == maxPage ? data.GetTupleNum() % PAGE_SIZE : PAGE_SIZE);
		for (indexRow = 0; indexRow < upperBound; indexRow++)
		{
			for (indexColumn = 0; indexColumn < ui.tableWidget_generalized->columnCount(); indexColumn++)
			{
				p_item = new QTableWidgetItem;
				p_item->setText( generalizor.GetValueQstr( vec_order[(generalizedCurrentPage - 1) * PAGE_SIZE + indexRow], indexColumn ) );
				ui.tableWidget_generalized->setItem( indexRow, indexColumn, p_item );
			}
		}
	}
}

void myMainWindow::revert( const int indexRow )
{
	/* -------------- find the nearest generalization step -------------- */
	int indexGen;
	for ( indexGen = indexRow; indexGen >= 0; indexGen-- )
	{
		if ( vec_log[ indexGen ].type_log == anony::GENERALIZATION )
		{
			break;
		}
	}

	// undo all the steps
	if ( indexGen == -1 )
	{
		generalizor.Resume( vec_order );
		for ( int indexTemp = 0; indexTemp < generalizor.m_vecStatus.size(); indexTemp++)
		{
			generalizor.m_vecStatus[indexTemp] = anony::NORMAL;
		}
		generalizor.m_bRootDirty = true;

		updateDeleted();

		ui.tableWidget_generalized->setRowCount( 0 );
		ui.tableWidget_deleted->setRowCount( 0 );

		state = anony::AFTER_HIERARCHY;
		setState();

		return;
	}

	myLog genLog = vec_log[ indexGen ];

	/* -------------- initialize all the parameters to default -------------- */
	ui.doubleSpinBox_threshold->setSpecialValueText( 0 );
	ui.horizontalSlider_threshold->setValue( 0 );
	ui.lineEdit_number_deleted->clear();
	ui.spinBox_l->setValue( 0 );
	ui.doubleSpinBox_c->setValue( 0 );
	ui.spinBox_bk->setValue( 0 );

	/* -------------- make the generalization based on the log -------------- */
	int indexTuple = 0;
	for ( int indexDel = 0; indexDel < genLog.para_vec_del.size(); indexDel++ )
	{
		int indexTemp;
		for ( indexTemp = indexTuple; indexTemp < genLog.para_vec_del[indexDel]; indexTemp++)
		{
			generalizor.m_vecStatus[indexTemp] = anony::NORMAL;
		}
		generalizor.m_vecStatus[indexTemp] = anony::DELETED;
		indexTuple = indexTemp + 1;
	}

	int indexTemp;
	for ( indexTemp = (genLog.para_vec_del.empty() ? 0 : genLog.para_vec_del[genLog.para_vec_del.size() - 1] + 1); indexTemp < generalizor.m_vecStatus.size(); indexTemp++)
	{
		generalizor.m_vecStatus[indexTemp] = anony::NORMAL;
	}

	vec_del = genLog.para_vec_del;
	generalizor.Resume( vec_order );
	generalizor.Clean( vec_order );
	generalizor.m_bRootDirty = true;

	if ( genLog.type_verifier == anony::DIVERSITY )
	{
		ui.spinBox_l->setValue( genLog.para_l );
		ui.doubleSpinBox_c->setValue( genLog.para_c );
		ui.tabWidget_verifier->setCurrentWidget ( ui.tab_diversity );
	}
	else if ( genLog.type_verifier == anony::CLOSENESS )
	{
		ui.doubleSpinBox_t->setValue( genLog.para_t );
		ui.tabWidget_verifier->setCurrentWidget( ui.tab_closeness );
	}

	generalize( true );
	updateDeleted();

	/*-------------- follow the other steps one-by-one -------------- */
	for ( int indexStep = indexGen + 1; indexStep <= indexRow; indexStep++ )
	{
		if ( vec_log[indexStep].type_log == anony::EVALUATE )
		{
			ui.spinBox_bk->setValue(vec_log[indexStep].para_piece_sa);
			for(int rowIndex = 0; rowIndex < ui.tableWidget_bk->rowCount(); rowIndex++)
			{
				((QCheckBox *) ui.tableWidget_bk->cellWidget( rowIndex, 0 ))->setChecked(false);
			}
			for(int rowIndex = 0; rowIndex < vec_log[indexStep].para_vec_bg_qi.size(); rowIndex++)
			{
				((QCheckBox *) ui.tableWidget_bk->cellWidget( vec_log[indexStep].para_vec_bg_qi[rowIndex], 0 ))->setChecked(true);
			}
			evaluateRisk( true );
		}
		else if ( vec_log[indexStep].type_log == anony::DELETE )
		{
			ui.lineEdit_number_deleted->setText( QString::number(vec_log[indexStep].para_num_del) );
			deleteTuple( true );
		}
	}
}


void myMainWindow::genHisto( const vector<long>& vecCnt, float max_risk, float min_risk, QGraphicsScene& scnHisto )
{
	const long bin_cnt = vecCnt.size();
	const long max_cnt = *max_element( vecCnt.begin(), vecCnt.end() );
	const long sum_cnt = data.GetTupleNum();
	float max_ratio = float(max_cnt) / sum_cnt;
	float up_bound;

	// set upper bound in the y-axis
	if (float( max_ratio ) > 0.5)
	{
		up_bound = 1;
	}
	else if (float( max_ratio ) > 0.25)
	{
		up_bound = 0.5;
	}
	else if (float( max_ratio ) > 0.2)
	{
		up_bound = 0.25;
	}
	else if (float( max_ratio ) > 0.1)
	{
		up_bound = 0.2;
	}
	else if (float( max_ratio ) > 0.05)
	{
		up_bound = 0.1;
	}
	else if (float( max_ratio ) > 0.04)
	{
		up_bound = 0.05;
	}
	else if (float( max_ratio ) > 0.03)
	{
		up_bound = 0.04;
	}
	else if (float( max_ratio ) > 0.02)
	{
		up_bound = 0.03;
	}
	else if (float( max_ratio ) > 0.01)
	{
		up_bound = 0.02;
	}
	else if (float( max_ratio ) > 0.005)
	{
		up_bound = 0.01;
	}
	else if (float( max_ratio ) > 0.001)
	{
		up_bound = 0.005;
	}
	else if (float( max_ratio ) > 0.0005)
	{
		up_bound = 0.001;
	}
	else
	{
		up_bound = 0.0005;
	}

	//set font for text
	QFont text_font;
	text_font.setBold( true );
	text_font.setStyleHint(QFont::Times);

	//add text "risk" and "percentage"
	QGraphicsTextItem *textRisk = scnHisto.addText("Risk ( % )", text_font);
	textRisk->setPos( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + (bin_cnt * BIN_WIDTH) / 2 - 25, TOP_MARGIN + BIN_HEIGHT * bin_cnt + X_SUB_LEN + BOTTOM_MARGIN);
	QGraphicsTextItem *textPercentage = scnHisto.addText("Fraction of Tuples ( % )", text_font);
	textPercentage->setPos( LEFT_MARGIN, TOP_MARGIN + BIN_HEIGHT * bin_cnt / 2  + 90);
	textPercentage->rotate(270);

	//set histogram brush and pen settings
	QBrush histo_brush( Qt::darkGreen, Qt::SolidPattern );
	QPen histo_pen( Qt::SolidLine );
	histo_pen.setJoinStyle( Qt::RoundJoin );
	histo_pen.setCapStyle( Qt::SquareCap );
	histo_pen.setWidth( 2 );

	//add axis x and y
	QLineF y_axis_line;
	QGraphicsLineItem *y_axis = scnHisto.addLine( y_axis_line );
	y_axis->setPen( histo_pen );
	y_axis->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN + bin_cnt * BIN_HEIGHT );
	QLineF x_axis_line;
	QGraphicsLineItem *x_axis = scnHisto.addLine( x_axis_line );
	x_axis->setPen( histo_pen );
	x_axis->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN + bin_cnt * BIN_HEIGHT, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + bin_cnt * BIN_WIDTH, TOP_MARGIN + bin_cnt * BIN_HEIGHT );

	long i;
	for(i = 0; i <= X_AXIS_INTERVAL; i++)
	{
		QLineF x_sub_line;
		QGraphicsLineItem *x_sub = scnHisto.addLine( x_sub_line );
		x_sub->setPen( histo_pen );
		x_sub->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + bin_cnt * BIN_WIDTH * i / X_AXIS_INTERVAL, TOP_MARGIN + bin_cnt * BIN_HEIGHT, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + bin_cnt * BIN_WIDTH * i / X_AXIS_INTERVAL, TOP_MARGIN + bin_cnt * BIN_HEIGHT + X_SUB_LEN );

		QString x_label = QString::number( ( min_risk + ( max_risk - min_risk ) / X_AXIS_INTERVAL * i ) * 100 );
		QGraphicsTextItem *text_label = scnHisto.addText( x_label, text_font );
		text_label->setPos( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + bin_cnt * BIN_WIDTH * i / X_AXIS_INTERVAL - 5, TOP_MARGIN + bin_cnt * BIN_HEIGHT + X_SUB_LEN );
	}

	for(i = 0; i <= Y_AXIS_INTERVAL; i++)
	{
		QLineF y_sub_line;
		QGraphicsLineItem *y_sub = scnHisto.addLine( y_sub_line );
		y_sub->setPen( histo_pen );
		y_sub->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN + bin_cnt * BIN_HEIGHT - bin_cnt * BIN_HEIGHT * i / Y_AXIS_INTERVAL, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN - Y_SUB_LEN, TOP_MARGIN + bin_cnt * BIN_HEIGHT - bin_cnt * BIN_HEIGHT * i / Y_AXIS_INTERVAL );

		QString y_label = QString::number((double) up_bound / Y_AXIS_INTERVAL * i * 100);
		QGraphicsTextItem *text_label = scnHisto.addText(y_label, text_font);
		text_label->setPos( PERCENTAGE_TEXT_SIZE - Y_SUB_LEN - NUMBER_LEN, TOP_MARGIN + bin_cnt * BIN_HEIGHT - bin_cnt * BIN_HEIGHT * i / Y_AXIS_INTERVAL - 5);
	}

	int bound_bin = 0;
	for (i = 0; i < bin_cnt; ++i)
	{
		QRectF q_rectangle( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + i * BIN_WIDTH, TOP_MARGIN + bin_cnt * BIN_HEIGHT - bin_cnt * BIN_HEIGHT * float( vecCnt[i] ) / float( up_bound * sum_cnt ), BIN_WIDTH, bin_cnt * BIN_HEIGHT * float( vecCnt[i] ) / float( up_bound * sum_cnt ));
		
		scnHisto.addRect( q_rectangle, histo_pen, histo_brush );

		if ( vecCnt[i] != 0 )
		{
			bound_bin = i + 1;
		}
	}

	// set threshold bar originally to 0
	QPen threshold_pen( Qt::SolidLine );
	threshold_pen.setJoinStyle( Qt::RoundJoin );
	threshold_pen.setCapStyle( Qt::SquareCap );
	threshold_pen.setWidth( 3 );
	threshold_pen.setColor(Qt::red);

	QLineF threshold_line;
	QGraphicsLineItem *threshold = scnHisto.addLine( threshold_line );
	threshold->setPen( threshold_pen );
	threshold->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + bound_bin * BIN_WIDTH, TOP_MARGIN, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + bound_bin * BIN_WIDTH, TOP_MARGIN + bin_cnt * BIN_HEIGHT );
	threshold->setZValue( 10000 );
	thresHold = threshold;
	maxRisk = max_risk;
	minRisk = min_risk;

	ui.doubleSpinBox_threshold->setRange( min_risk, max_risk );
	ui.doubleSpinBox_threshold->setValue( min_risk + ( max_risk - min_risk ) / bin_cnt * bound_bin );
	ui.horizontalSlider_threshold->setRange( min_risk * 100, max_risk * 100 );
}

void myMainWindow::plotMarginalDensity()
{
	const long dimension = ui.comboBox_dimension->currentIndex();

	long lengthVec;
	vector<float> vecOri;
	vector<float> vecAnony;
	vector<QString> rVecName;

	generalizor.GetMarginalDensity( dimension, lengthVec, vecOri, vecAnony, rVecName );

	/* ---- original density histogram ---- */
	QGraphicsScene* ori_old_scene = ui.graphicsView_original_2->scene();
	QGraphicsScene* ori_new_scene = new QGraphicsScene;

	float bin_cnt = vecOri.size();
	float max_cnt = *max_element( vecOri.begin(), vecOri.end() );
	float step = MAX_WIDGTH / bin_cnt;

	//set font for text
	QFont text_font;
	text_font.setBold( true );
	text_font.setStyleHint(QFont::Times);

	//set histogram brush and pen settings
	QBrush histo_brush( Qt::darkBlue, Qt::SolidPattern );
	QPen histo_pen( Qt::SolidLine );
	histo_pen.setJoinStyle( Qt::RoundJoin );
	histo_pen.setCapStyle( Qt::SquareCap );
	histo_pen.setWidth( 5 );

	//add axis x and y
	QLineF y_axis_line;
	QGraphicsLineItem *y_axis = ori_new_scene->addLine( y_axis_line );
	y_axis->setPen( histo_pen );
	y_axis->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN + MAX_HEIGHT );
	QLineF x_axis_line;
	QGraphicsLineItem *x_axis = ori_new_scene->addLine( x_axis_line );
	x_axis->setPen( histo_pen );
	x_axis->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN + MAX_HEIGHT, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + MAX_WIDGTH, TOP_MARGIN + MAX_HEIGHT );

	for (int i = 0; i < lengthVec; ++i)
	{
		QRectF q_rectangle( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + i * step, TOP_MARGIN + MAX_HEIGHT - MAX_HEIGHT * float( vecOri[i] ) / float( max_cnt ), step, MAX_HEIGHT * float( vecOri[i] ) / float( max_cnt ));

		QPen temp_pen;
		temp_pen.setColor(histo_brush.color());
		ori_new_scene->addRect( q_rectangle, temp_pen, histo_brush );
	}

	ui.graphicsView_original_2->setScene( ori_new_scene );
	QRectF boundRec = ori_new_scene->sceneRect();
	ui.graphicsView_original_2->fitInView( boundRec );

	if ( ori_old_scene != NULL )
	{
		delete ori_old_scene;
	}

	/* ---- generalized density histogram ---- */
	ori_old_scene = ui.graphicsView_generalized_2->scene();
	ori_new_scene = new QGraphicsScene;

	bin_cnt = vecOri.size();
	max_cnt = *max_element( vecOri.begin(), vecOri.end() );

	//set font for text
	text_font.setBold( true );
	text_font.setStyleHint(QFont::Times);

	//set histogram brush and pen settings
	QBrush histo_brush_2( Qt::darkRed, Qt::SolidPattern );
	histo_pen.setJoinStyle( Qt::RoundJoin );
	histo_pen.setCapStyle( Qt::SquareCap );
	histo_pen.setWidth( 5 );

	//add axis x and y
	y_axis = ori_new_scene->addLine( y_axis_line );
	y_axis->setPen( histo_pen );
	y_axis->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN + MAX_HEIGHT );
	x_axis = ori_new_scene->addLine( x_axis_line );
	x_axis->setPen( histo_pen );
	x_axis->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN + MAX_HEIGHT, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + MAX_WIDGTH, TOP_MARGIN + MAX_HEIGHT );

	for (int i = 0; i < lengthVec; ++i)
	{
		QRectF q_rectangle( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + i * step, TOP_MARGIN + MAX_HEIGHT - MAX_HEIGHT * float( vecAnony[i] ) / float( max_cnt ), step, MAX_HEIGHT * float( vecAnony[i] ) / float( max_cnt ));

		QPen temp_pen;
		temp_pen.setColor(histo_brush_2.color());
		ori_new_scene->addRect( q_rectangle, temp_pen, histo_brush_2 );
	}

	ui.graphicsView_generalized_2->setScene( ori_new_scene );
	boundRec = ori_new_scene->sceneRect();
	ui.graphicsView_generalized_2->fitInView( boundRec );

	if ( ori_old_scene != NULL )
	{
		delete ori_old_scene;
	}
}

void myMainWindow::plotJointDensity()
{
	const long x_dimension = ui.comboBox_x_dimension_2->currentIndex();
	const long y_dimension = ui.comboBox_y_dimension_2->currentIndex();

	long widthVec;
	long heightVec;
	vector<float> vecOri;
	vector<float> vecAnony;
	vector<QString> rVecNameX;
	vector<QString> rVecNameY;

	if( x_dimension == y_dimension ){

		QMessageBox::information(this, "Message", "Two attributes are the same, please choose different attributes.");
	}
	else{

		float countMax;
		generalizor.GetJointDensity( x_dimension, y_dimension, widthVec, heightVec, vecOri, vecAnony, rVecNameX, rVecNameY, countMax );

		int i, j;

		// original density graph
		QGraphicsScene* ori_old_scene = ui.graphicsView_original->scene();
		QGraphicsScene* ori_new_scene = new QGraphicsScene;
		
		for ( i = 0; i < widthVec; ++i )
		{
			for ( j = 0; j < heightVec; ++j )
			{
				QRectF q_rectangle( i, j, 1, 1 );

				QColor density_color;

				// Old Color Scheme					
				//density_color.setHsv( (int) ((1 - vecOri[ j * widthVec + i]) * 240), 255, 200 );

				// New Color Scheme
				if (vecOri[j * widthVec + i] < 0.1)
				{
					density_color.setHsv( 238, 200, 2 * (56 + int( 44 * vecOri[j * widthVec + i] )) );
				}
				else if (vecOri[j * widthVec + i] < 0.35)
				{
					density_color.setHsv( 238 - int( 59 * (vecOri[j * widthVec + i] - 0.1) / 0.25 ), 200, 200 );
				}
				else if (vecOri[j * widthVec + i] < 0.6)
				{
					density_color.setHsv( 179 - int( 120 * (vecOri[j * widthVec + i] - 0.35) / 0.25 ), 200, 200 );
				}
				else if (vecOri[j * widthVec + i] < 0.85)
				{
					density_color.setHsv( 59 - int( 59 * (vecOri[j * widthVec + i] - 0.6) / 0.25 ), 200, 200 );
				}
				else
				{
					density_color.setHsv( 0, 200, 200, 2 * (100 - int( 50 * (vecOri[j * widthVec + i] - 0.85) / 0.15 ) ) );
				}

				QBrush density_brush( density_color, Qt::SolidPattern );
				QPen density_pen(density_color);

				ori_new_scene->addRect( q_rectangle, density_pen, density_brush );
			}		
		}

		ui.graphicsView_original->setScene( ori_new_scene );
		QRectF boundRec = ori_new_scene->sceneRect();
		ui.graphicsView_original->fitInView( boundRec );

		if ( ori_old_scene != NULL )
		{
			delete ori_old_scene;
		}

		// generalized density graph
		QGraphicsScene* gen_old_scene = ui.graphicsView_generalized->scene();
		QGraphicsScene* gen_new_scene = new QGraphicsScene;

		for ( i = 0; i < widthVec; ++i )
		{
			for ( j = 0; j < heightVec; ++j )
			{

				QRectF q_rectangle( i, j, 1, 1 );

				QColor density_color;

				// Old Color Scheme
				//density_color.setHsv( (int) ((1 - vecAnony[ j * widthVec + i]) * 240), 255, 200 );

				// New Color Scheme
				if (vecAnony[j * widthVec + i] < 0.1)
				{
					density_color.setHsv( 238, 200, 2 * (56 + int( 44 * vecAnony[j * widthVec + i] )) );
				}
				else if (vecAnony[j * widthVec + i] < 0.35)
				{
					density_color.setHsv( 238 - int( 59 * (vecAnony[j * widthVec + i] - 0.1) / 0.25 ), 200, 200 );
				}
				else if (vecAnony[j * widthVec + i] < 0.6)
				{
					density_color.setHsv( 179 - int( 120 * (vecAnony[j * widthVec + i] - 0.35) / 0.25 ), 200, 200 );
				}
				else if (vecAnony[j * widthVec + i] < 0.85)
				{
					density_color.setHsv( 59 - int( 59 * (vecAnony[j * widthVec + i] - 0.6) / 0.25 ), 200, 200 );
				}
				else
				{
					density_color.setHsv( 0, 200, 200, 2 * (100 - int( 50 * (vecAnony[j * widthVec + i] - 0.85) / 0.15 ) ) );
				}

				QBrush density_brush( density_color, Qt::SolidPattern );
				QPen density_pen(density_color);

				gen_new_scene->addRect( q_rectangle, density_pen, density_brush );
			}		
		}

		ui.graphicsView_generalized->setScene( gen_new_scene );
		boundRec = gen_new_scene->sceneRect();
		ui.graphicsView_generalized->fitInView( boundRec );

		if ( gen_old_scene != NULL )
		{
			delete gen_old_scene;
		}
		
	}
}

void myMainWindow::plotMarginalDensityDifference()
{
	const long dimension = ui.comboBox_dimension->currentIndex();

	long lengthVec;
	vector<float> vecOri;
	vector<float> vecAnony;
	vector<QString> rVecName;

	generalizor.GetMarginalDensity( dimension, lengthVec, vecOri, vecAnony, rVecName );

	/* ---- overlay density curve ---- */
	QGraphicsScene* ori_old_scene = ui.graphicsView_original_2->scene();
	QGraphicsScene* ori_new_scene = new QGraphicsScene;

	float bin_cnt = vecOri.size();
	float max_cnt = *max_element( vecOri.begin(), vecOri.end() );
	float step = MAX_WIDGTH / bin_cnt;

	//set font for text
	QFont text_font;
	text_font.setBold( true );
	text_font.setStyleHint(QFont::Times);

	//set histogram brush and pen settings
	QBrush histo_brush( Qt::blue, Qt::SolidPattern );
	QBrush histo_brush_2( Qt::red, Qt::SolidPattern );
	QPen histo_pen( Qt::SolidLine );
	histo_pen.setJoinStyle( Qt::RoundJoin );
	histo_pen.setCapStyle( Qt::SquareCap );
	histo_pen.setWidth( 5 );

	//add axis x and y
	QLineF y_axis_line;
	QGraphicsLineItem *y_axis = ori_new_scene->addLine( y_axis_line );
	y_axis->setPen( histo_pen );
	y_axis->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN + MAX_HEIGHT );
	QLineF x_axis_line;
	QGraphicsLineItem *x_axis = ori_new_scene->addLine( x_axis_line );
	x_axis->setPen( histo_pen );
	x_axis->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN + MAX_HEIGHT, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + MAX_WIDGTH, TOP_MARGIN + MAX_HEIGHT );

	int prev_ori = TOP_MARGIN + MAX_HEIGHT, prev_anony = TOP_MARGIN + MAX_HEIGHT;

	for (int i = 0; i < lengthVec; ++i)
	{
		QPen temp_pen;
		temp_pen.setColor(histo_brush.color());

		QLineF h_line_ori;
		QGraphicsLineItem *hor_line_ori = ori_new_scene->addLine( h_line_ori );
		hor_line_ori->setPen( temp_pen );
		hor_line_ori->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + i * step, TOP_MARGIN + MAX_HEIGHT - MAX_HEIGHT * float( vecOri[i] ) / float( max_cnt ), PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + (i + 1) * step, TOP_MARGIN + MAX_HEIGHT - MAX_HEIGHT * float( vecOri[i] ) / float( max_cnt ) );

		QLineF v_line_ori;
		QGraphicsLineItem *ver_line_ori = ori_new_scene->addLine( h_line_ori );
		ver_line_ori->setPen( temp_pen );
		ver_line_ori->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + i * step, TOP_MARGIN + MAX_HEIGHT - MAX_HEIGHT * float( vecOri[i] ) / float( max_cnt ), PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + i * step, prev_ori );

		prev_ori = TOP_MARGIN + MAX_HEIGHT - MAX_HEIGHT * float( vecOri[i] ) / float( max_cnt );

		temp_pen.setColor(histo_brush_2.color());

		QLineF h_line_anony;
		QGraphicsLineItem *hor_line_anony = ori_new_scene->addLine( h_line_anony );
		hor_line_anony->setPen( temp_pen );
		hor_line_anony->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + i * step, TOP_MARGIN + MAX_HEIGHT - MAX_HEIGHT * float( vecAnony[i] ) / float( max_cnt ), PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + (i + 1) * step, TOP_MARGIN + MAX_HEIGHT - MAX_HEIGHT * float( vecAnony[i] ) / float( max_cnt ) );

		QLineF v_line_anony;
		QGraphicsLineItem *ver_line_anony = ori_new_scene->addLine( h_line_anony );
		ver_line_anony->setPen( temp_pen );
		ver_line_anony->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + i * step, TOP_MARGIN + MAX_HEIGHT - MAX_HEIGHT * float( vecAnony[i] ) / float( max_cnt ), PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + i * step, prev_anony );

		prev_anony = TOP_MARGIN + MAX_HEIGHT - MAX_HEIGHT * float( vecAnony[i] ) / float( max_cnt );
	}

	ui.graphicsView_original_2->setScene( ori_new_scene );
	QRectF boundRec = ori_new_scene->sceneRect();
	ui.graphicsView_original_2->fitInView( boundRec );

	if ( ori_old_scene != NULL )
	{
		delete ori_old_scene;
	}

	/* ---- density difference histogram ---- */
	ori_old_scene = ui.graphicsView_generalized_2->scene();
	ori_new_scene = new QGraphicsScene;

	bin_cnt = vecOri.size();
	max_cnt = *max_element( vecOri.begin(), vecOri.end() );

	//set font for text
	text_font.setBold( true );
	text_font.setStyleHint(QFont::Times);

	//set histogram brush and pen settings
	histo_pen.setJoinStyle( Qt::RoundJoin );
	histo_pen.setCapStyle( Qt::SquareCap );
	histo_pen.setWidth( 5 );

	//add axis x and y
	y_axis = ori_new_scene->addLine( y_axis_line );
	y_axis->setPen( histo_pen );
	y_axis->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN + MAX_HEIGHT );
	x_axis = ori_new_scene->addLine( x_axis_line );
	x_axis->setPen( histo_pen );
	x_axis->setLine( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN, TOP_MARGIN + MAX_HEIGHT / 2, PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + MAX_WIDGTH, TOP_MARGIN + MAX_HEIGHT / 2 );

	for (int i = 0; i < lengthVec; ++i)
	{
		QRectF q_rectangle( PERCENTAGE_TEXT_SIZE + LEFT_MARGIN + i * step, TOP_MARGIN + MAX_HEIGHT / 2 - MAX_HEIGHT * float( vecAnony[i] - vecOri[i] ) / float( max_cnt ), step, MAX_HEIGHT * float( vecAnony[i] - vecOri[i] ) / float( max_cnt ));

		QPen temp_pen;
		temp_pen.setColor(histo_brush_2.color());
		ori_new_scene->addRect( q_rectangle, temp_pen, histo_brush_2 );
	}

	ui.graphicsView_generalized_2->setScene( ori_new_scene );
	boundRec = ori_new_scene->sceneRect();
	ui.graphicsView_generalized_2->fitInView( boundRec );

	if ( ori_old_scene != NULL )
	{
		delete ori_old_scene;
	}
}


void myMainWindow::calcChiSquareTest()
{
	const long x_dimension = ui.comboBox_x_dimension->currentIndex();
	const long y_dimension = ui.comboBox_y_dimension->currentIndex();

	long widthVec;
	long heightVec;
	vector<float> vecOri;
	vector<float> vecAnony;
	vector<QString> rVecNameX;
	vector<QString> rVecNameY;

	if ( generalizor.m_pMicrodata->GetVecMeta()[x_dimension].m_nType != anony::CATEGORICAL )
	{
		ui.lineEdit_gof->hide();
		ui.label_gof->hide();
	}
	else if ( generalizor.m_pMicrodata->GetVecMeta()[y_dimension].m_nType != anony::CATEGORICAL )
	{
		ui.lineEdit_gof->hide();
		ui.label_gof->hide();
	}
	else
	{
		const vector<Metadata> &vectorMeta = data.GetVecMeta();
		float countMax;

		generalizor.GetJointDensity( x_dimension, y_dimension, widthVec, heightVec, vecOri, vecAnony, rVecNameX, rVecNameY, countMax );

		// compute X^2 statistic value
		float chi2 = 0;

		for ( int i = 0; i < widthVec; ++i )
		{
			for ( int j = 0; j < heightVec; ++j )
			{
				if (  vecAnony[j * widthVec + i] > 0 )
				{
					chi2 += (float) ( vecOri[j * widthVec + i] * countMax - vecAnony[j * widthVec + i] * countMax ) * ( vecOri[j * widthVec + i] * countMax - vecAnony[j * widthVec + i] * countMax ) / ( vecAnony[j * widthVec + i] * countMax ); 
				}
			}
		}

		// compute p-value
 		int a = ( widthVec - 1 ) * ( heightVec - 1 );
 		float pvalue = chisquarecdistribution(a, chi2);
		ui.lineEdit_gof->setText(QString::number(pvalue));
	}
}

void myMainWindow::createContingencyTable()
{
	const long x_dimension = ui.comboBox_x_dimension->currentIndex();
	const long y_dimension = ui.comboBox_y_dimension->currentIndex();

	long widthVec;
	long heightVec;
	vector<float> vecOri;
	vector<float> vecAnony;
	vector<QString> rVecNameX;
	vector<QString> rVecNameY;

	if( x_dimension == y_dimension ){

		QMessageBox::information(this, "Message", "Two attributes are the same, please choose different attributes.");
	}
	else{

		const vector<Metadata> &vectorMeta = data.GetVecMeta();
		float countMax;

		generalizor.GetJointDensity( x_dimension, y_dimension, widthVec, heightVec, vecOri, vecAnony, rVecNameX, rVecNameY, countMax );

		ui.tableWidget_contingency_generalized->clearContents();
		ui.tableWidget_contingency_original->clearContents();

		ui.tableWidget_contingency_original->setColumnCount( widthVec );
		ui.tableWidget_contingency_original->setRowCount( heightVec );
		ui.tableWidget_contingency_original->setEditTriggers( QAbstractItemView::NoEditTriggers );

		ui.tableWidget_contingency_generalized->setColumnCount( widthVec );
		ui.tableWidget_contingency_generalized->setRowCount( heightVec );
		ui.tableWidget_contingency_generalized->setEditTriggers( QAbstractItemView::NoEditTriggers );

		// add column and row attribute names
		int indexColumn, indexCell, indexRow;
		QStringList temp_list;

		for( indexRow = 0; indexRow < rVecNameX.size(); indexRow++ )
		{
			temp_list.append(rVecNameX[ indexRow ]);
		}

		ui.tableWidget_contingency_original->setHorizontalHeaderLabels(temp_list);
		ui.tableWidget_contingency_generalized->setHorizontalHeaderLabels(temp_list);
		temp_list.clear();

		for( indexColumn = 0; indexColumn < rVecNameY.size(); indexColumn++ )
		{
			temp_list.append(rVecNameY[ indexColumn ]);
		}

		ui.tableWidget_contingency_original->setVerticalHeaderLabels(temp_list);
		ui.tableWidget_contingency_generalized->setVerticalHeaderLabels(temp_list);

		// insert cell densities
		QTableWidgetItem* p_item_original;
		QTableWidgetItem* p_item_generalized;

		for ( int i = 0; i < widthVec; ++i )
		{
			for ( int j = 0; j < heightVec; ++j )
			{
				p_item_original = new QTableWidgetItem;
				p_item_generalized = new QTableWidgetItem;

				p_item_original->setText( QString::number( (int) (vecOri[ j * widthVec + i] * countMax) ));
				ui.tableWidget_contingency_original->setItem( j, i, p_item_original );

				p_item_generalized->setText( QString::number( (int) (vecAnony[ j * widthVec + i]  * countMax) ));
				ui.tableWidget_contingency_generalized->setItem( j, i, p_item_generalized );
			}		
		}
		ui.tableWidget_contingency_original->resizeColumnsToContents();
		ui.tableWidget_contingency_generalized->resizeColumnsToContents();
	}
}

void myMainWindow::nextPage()
{

	if(ui.tabWidget_data->currentWidget() == ui.tab_original)
	{
		nextPageOriginal();
	}
	else if(ui.tabWidget_data->currentWidget() == ui.tab_generalized)
	{
		nextPageGeneralized();
	}
	else
	{
		nextPageDeleted();
	}
}

void myMainWindow::lastPage()
{
	if(ui.tabWidget_data->currentWidget() == ui.tab_original)
	{
		lastPageOriginal();
	}
	else if(ui.tabWidget_data->currentWidget() == ui.tab_generalized)
	{
		lastPageGeneralized();
	}
	else
	{
		lastPageDeleted();
	}
}


void myMainWindow::nextPageOriginal()
{
	const int columnCount = ui.tableWidget_original->columnCount();
	const vector<UnitValue> &tuple = data.GetVecTuple();
	int indexColumn, indexRow;
	QTableWidgetItem* p_item;

	originalCurrentPage++;

	int upperBound = (originalCurrentPage == maxPage ? vec_order.size() % PAGE_SIZE : PAGE_SIZE);
	ui.tableWidget_original->setRowCount( upperBound );

	for(indexRow = 0; indexRow < upperBound; indexRow++){

		for (indexColumn = 0; indexColumn < columnCount - 1; indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( data.GetValueQstr( vec_order[(originalCurrentPage - 1) * PAGE_SIZE + indexRow], indexColumn ) );
			ui.tableWidget_original->setItem( indexRow, indexColumn, p_item );
		}

		// add disclosure risk
		if (generalizor.m_vecRisk.empty())
		{
			p_item = new QTableWidgetItem;
			p_item->setText( "N/A" );
			ui.tableWidget_original->setItem( indexRow, (columnCount - 1), p_item );
		}
		else 
		{
			p_item = new QTableWidgetItem;
			p_item->setText( QString::number( (int) (generalizor.m_vecRisk[vec_order[(originalCurrentPage - 1) * PAGE_SIZE + indexRow] ] * 100)) + "%"  );
			ui.tableWidget_original->setItem( indexRow, (columnCount - 1), p_item );
		}
	}

	if (originalCurrentPage == maxPage)
	{
		ui.pushButton_next->setDisabled(true);
	}
	if (originalCurrentPage == 2)
	{
		ui.pushButton_last->setDisabled(false);
	}
}

void myMainWindow::lastPageOriginal()
{
	const int columnCount = ui.tableWidget_original->columnCount();
	const vector<UnitValue> &tuple = data.GetVecTuple();
	int indexColumn, indexRow;
	QTableWidgetItem* p_item;

	originalCurrentPage--;

	ui.tableWidget_original->setRowCount( PAGE_SIZE );
	for(indexRow = 0; indexRow < PAGE_SIZE; indexRow++){

		for (indexColumn = 0; indexColumn < columnCount - 1; indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( data.GetValueQstr( vec_order[(originalCurrentPage - 1) * PAGE_SIZE + indexRow], indexColumn ) );
			ui.tableWidget_original->setItem( indexRow, indexColumn, p_item );
		}

		// add disclosure risk
		if (generalizor.m_vecRisk.empty())
		{
			p_item = new QTableWidgetItem;
			p_item->setText( "N/A" );
			ui.tableWidget_original->setItem( indexRow, (columnCount - 1), p_item );
		}
		else 
		{
			p_item = new QTableWidgetItem;
			p_item->setText( QString::number( (int) (generalizor.m_vecRisk[vec_order[(originalCurrentPage - 1) * PAGE_SIZE + indexRow] ] * 100)) + "%"  );
			ui.tableWidget_original->setItem( indexRow, (columnCount - 1), p_item );
		}
	}

	if (originalCurrentPage == 1)
	{
		ui.pushButton_last->setDisabled(true);
	}
	if(originalCurrentPage == maxPage - 1)
	{
		ui.pushButton_next->setDisabled(false);
	}
}

void myMainWindow::nextPageGeneralized()
{
	const int columnCount = ui.tableWidget_generalized->columnCount();
	const vector<UnitValue> &tuple = data.GetVecTuple();
	int indexColumn, indexRow;
	QTableWidgetItem* p_item;

	generalizedCurrentPage++;

	int upperBound = (generalizedCurrentPage == maxPage ? vec_order.size() % PAGE_SIZE : PAGE_SIZE);
	ui.tableWidget_generalized->setRowCount( upperBound );

	for(indexRow = 0; indexRow < upperBound; indexRow++){

		for (indexColumn = 0; indexColumn < columnCount - 1; indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( generalizor.GetValueQstr( vec_order[(generalizedCurrentPage - 1) * PAGE_SIZE + indexRow], indexColumn ) );
			ui.tableWidget_generalized->setItem( indexRow, indexColumn, p_item );
		}
	}

	if (generalizedCurrentPage == maxPage)
	{
		ui.pushButton_next->setDisabled(true);
	}
	if (generalizedCurrentPage == 2)
	{
		ui.pushButton_last->setDisabled(false);
	}
}

void myMainWindow::lastPageGeneralized()
{
	const int columnCount = ui.tableWidget_generalized->columnCount();
	const vector<UnitValue> &tuple = data.GetVecTuple();
	int indexColumn, indexRow;
	QTableWidgetItem* p_item;

	generalizedCurrentPage--;

	ui.tableWidget_generalized->setRowCount( PAGE_SIZE );
	for(indexRow = 0; indexRow < PAGE_SIZE; indexRow++){

		for (indexColumn = 0; indexColumn < columnCount - 1; indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( generalizor.GetValueQstr( vec_order[(generalizedCurrentPage - 1) * PAGE_SIZE + indexRow], indexColumn ) );
			ui.tableWidget_generalized->setItem( indexRow, indexColumn, p_item );
		}
	}

	if (generalizedCurrentPage == 1)
	{
		ui.pushButton_last->setDisabled(true);
	}
	if(generalizedCurrentPage == maxPage - 1)
	{
		ui.pushButton_next->setDisabled(false);
	}
}

void myMainWindow::nextPageDeleted()
{
	const int columnCount = ui.tableWidget_deleted->columnCount();
	const vector<UnitValue> &tuple = data.GetVecTuple();
	int indexColumn, indexCell, indexRow;
	QTableWidgetItem* p_item;

	deletedCurrentPage++;

	int upperBound = ((vec_del.size() - ( deletedCurrentPage - 1 ) * PAGE_SIZE) < PAGE_SIZE) ? (vec_del.size() - ( deletedCurrentPage - 1 ) * PAGE_SIZE) : PAGE_SIZE;
	ui.tableWidget_deleted->setRowCount( upperBound );

	for(int indexRow = 0; indexRow < upperBound; indexRow++){

		for (int indexColumn = 0; indexColumn < columnCount - 1; indexColumn++ )
		{
			p_item = new QTableWidgetItem;
			p_item->setText( data.GetValueQstr( vec_del[(deletedCurrentPage - 1) * PAGE_SIZE + indexRow], indexColumn ) );
			ui.tableWidget_deleted->setItem( indexRow, indexColumn, p_item );
		}

		// add disclosure risk
		if (generalizor.m_vecRisk.empty())
		{
			p_item = new QTableWidgetItem;
			p_item->setText( "N/A" );
			ui.tableWidget_original->setItem( indexRow, (columnCount - 1), p_item );
		}
		else 
		{
			p_item = new QTableWidgetItem;
			p_item->setText( QString::number( (int) (generalizor.m_vecRisk[ vec_del[(deletedCurrentPage - 1) * PAGE_SIZE + indexRow] ] * 100)) + "%" );
			ui.tableWidget_original->setItem( indexRow, (columnCount - 1), p_item );
		}
	}
	
	if (deletedCurrentPage == maxDeletePage)
	{
		ui.pushButton_next->setDisabled(true);
	}
	if (deletedCurrentPage == 2)
	{
		ui.pushButton_last->setDisabled(false);
	}
}

void myMainWindow::lastPageDeleted()
{
	const int columnCount = ui.tableWidget_deleted->columnCount();
	const vector<UnitValue> &tuple = data.GetVecTuple();
	int indexColumn, indexCell, indexRow;
	QTableWidgetItem* p_item;

	deletedCurrentPage--;
	
	ui.tableWidget_deleted->setRowCount( PAGE_SIZE );

	int i = ui.tableWidget_deleted->columnCount();
	int j = ui.tableWidget_deleted->rowCount();

	for(int indexRow = 0; indexRow < PAGE_SIZE; indexRow++){

		for (int indexColumn = 0; indexColumn < columnCount - 1; indexColumn++ )
		{
			p_item = new QTableWidgetItem;
			p_item->setText( data.GetValueQstr( vec_del[(deletedCurrentPage - 1) * PAGE_SIZE + indexRow], indexColumn ) );
			ui.tableWidget_deleted->setItem( indexRow, indexColumn, p_item );
		}

		// add disclosure risk
		if (generalizor.m_vecRisk.empty())
		{
			p_item = new QTableWidgetItem;
			p_item->setText( "N/A" );
			ui.tableWidget_original->setItem( indexRow, (columnCount - 1), p_item );
		}
		else 
		{
			p_item = new QTableWidgetItem;
			p_item->setText( QString::number( (int) (generalizor.m_vecRisk[ vec_del[(deletedCurrentPage - 1) * PAGE_SIZE + indexRow] ] * 100)) + "%" );
			ui.tableWidget_original->setItem( indexRow, (columnCount - 1), p_item );
		}
	}

	if (deletedCurrentPage == 1)
	{
		ui.pushButton_last->setDisabled(true);
	}
	if (deletedCurrentPage == maxDeletePage - 1)
	{
		ui.pushButton_next->setDisabled(false);
	}
}

void myMainWindow::setCurrentPage( int index)
{
	// current page is original page
	if ( index == 0)
	{
		ui.tabWidget_data->setCurrentWidget( ui.tab_original );
		if (originalCurrentPage == maxPage)
		{
			ui.pushButton_next->setDisabled(true);
		}
		if (originalCurrentPage > 1)
		{
			ui.pushButton_last->setDisabled(false);
		}


		if (originalCurrentPage == 1)
		{
			ui.pushButton_last->setDisabled(true);
		}
		if (originalCurrentPage < maxPage)
		{
			ui.pushButton_next->setDisabled(false);
		}
	}
	// current page is generalized page
	else if ( index == 1)
	{
		ui.tabWidget_data->setCurrentWidget( ui.tab_generalized );
		if (generalizedCurrentPage == maxPage)
		{
			ui.pushButton_next->setDisabled(true);
		}
		if (generalizedCurrentPage > 1)
		{
			ui.pushButton_last->setDisabled(false);
		}


		if (generalizedCurrentPage == 1)
		{
			ui.pushButton_last->setDisabled(true);
		}
		if (generalizedCurrentPage < maxPage)
		{
			ui.pushButton_next->setDisabled(false);
		}
	}
	// current page is deleted page
	else
	{
		ui.tabWidget_data->setCurrentWidget( ui.tab_deleted );
		if (deletedCurrentPage == maxDeletePage)
		{
			ui.pushButton_next->setDisabled(true);
		}
		if (deletedCurrentPage > 1)
		{
			ui.pushButton_last->setDisabled(false);
		}


		if (deletedCurrentPage == 1)
		{
			ui.pushButton_last->setDisabled(true);
		}
		if (deletedCurrentPage < maxDeletePage)
		{
			ui.pushButton_next->setDisabled(false);
		}
	}
}

void myMainWindow::setCurrentVerifier( int index )
{
	// current verifier is L-Diversity
	if ( index == 0)
	{
		ui.tabWidget_verifier->setCurrentWidget( ui.tab_diversity );
		ui.label_bk->show();
		ui.spinBox_bk->show();
		ui.horizontalSlider_bk->show();
	}

	// current verifier is T-Closeness
	else if ( index == 1)
	{
		ui.tabWidget_verifier->setCurrentWidget( ui.tab_closeness );
		ui.label_bk->hide();
		ui.spinBox_bk->hide();
		ui.spinBox_bk->setValue( 0 );
		ui.horizontalSlider_bk->hide();
		ui.horizontalSlider_bk->setValue( 0 );
	}
}

//////////////////////////////////////////////////////////////////////////

/*
void myMainWindow::on_actionSelectSensitive_triggered()
{
selectSensitive->show();
selectSensitive->activateWindow();
}

void myMainWindow::readSensitive(const int sens)
{
const vector<Metadata> &vectorMeta = data.GetVecMeta();
sensAttr = sens;

ui.label_bk->setText(ui.label_bk->text() + " " + vectorMeta[sens].m_strName.c_str());
ui.label_bk->show();
ui.spinBox_bk->show();
ui.horizontalSlider_bk->show();


}

void myMainWindow::sortByRisk()
{
	generalizor.SortByRisk(vec_order, false);

	// update original and generalized table
	int indexRow, indexColumn;
	int upperBound = ( originalCurrentPage == maxPage ? data.GetTupleNum() % PAGE_SIZE : PAGE_SIZE);
	QTableWidgetItem *p_item;

	QBrush brush_gray;
	brush_gray.setColor( Qt::gray);
	
	for (indexRow = 0; indexRow < upperBound; indexRow++)
	{
		for (indexColumn = 0; indexColumn < ui.tableWidget_original->columnCount() - 1; indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( data.GetValueQstr( vec_order[(originalCurrentPage - 1) * PAGE_SIZE + indexRow], indexColumn ) );

			ui.tableWidget_original->setItem( indexRow, indexColumn, p_item );
		}

		p_item = new QTableWidgetItem;
		p_item->setText( QString::number( (int) (generalizor.m_vecRisk[vec_order[(originalCurrentPage - 1) * PAGE_SIZE + indexRow] ] * 100)) + "%"  );
		ui.tableWidget_original->setItem( indexRow, ui.tableWidget_original->columnCount() - 1, p_item );

		if (generalizor.m_vecStatus[vec_order[indexRow + (originalCurrentPage - 1) * PAGE_SIZE]] == anony::DELETED)
		{
			for (indexColumn = 0; indexColumn < ui.tableWidget_original->columnCount(); indexColumn++)
			{
				ui.tableWidget_original->item( indexRow, indexColumn )->setForeground( brush_gray );
			}
		}
	}

	upperBound = ( generalizedCurrentPage == maxPage ? data.GetTupleNum() % PAGE_SIZE : PAGE_SIZE);

	for (indexRow = 0; indexRow < upperBound; indexRow++)
	{
		for (indexColumn = 0; indexColumn < ui.tableWidget_generalized->columnCount(); indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( generalizor.GetValueQstr( vec_order[(generalizedCurrentPage - 1) * PAGE_SIZE + indexRow], indexColumn ) );

			ui.tableWidget_generalized->setItem( indexRow, indexColumn, p_item );
		}

		if (generalizor.m_vecStatus[vec_order[indexRow + (generalizedCurrentPage - 1) * PAGE_SIZE]] == anony::DELETED)
		{
			for (indexColumn = 0; indexColumn < ui.tableWidget_generalized->columnCount(); indexColumn++)
			{
				ui.tableWidget_generalized->item( indexRow, indexColumn )->setForeground( brush_gray );
			}
		}
	}

}


void myMainWindow::recoverAll()
{
	ui.horizontalSlider_threshold->setValue( 0 );
	ui.doubleSpinBox_threshold->setValue( 0 );
	ui.lineEdit_number_deleted->setText( QString::number( 0 ) );

	generalizor.RecoverAll();

	if ( sortIndex >= 0 )
	{
		if ( sortIndex == ui.tableWidget_original->columnCount() - 1 )
		{
			generalizor.SortByRisk(vec_order, !ascend);
			QMessageBox::information(this, "Message", "Here");
		}
		else
		{
			data.SortByAttr( sortIndex, vec_order, !ascend);
		}
	}

	generalizor.Clean( vec_order );

	// update deleted tables
	ui.tableWidget_deleted->clearContents();
	ui.tableWidget_deleted->setRowCount( 0 );

	// resume original and generalized tables
	int indexRow;
	int upperBound = ( vec_order.size() > PAGE_SIZE ? PAGE_SIZE : data.GetTupleNum());

	QTableWidgetItem* p_item;
	for(indexRow = 0; indexRow < upperBound; indexRow++){

		for (int indexColumn = 0; indexColumn < ui.tableWidget_original->columnCount() - 1; indexColumn++)
		{
			p_item = new QTableWidgetItem;
			p_item->setText( data.GetValueQstr( vec_order[indexRow], indexColumn ) );
			ui.tableWidget_original->setItem( indexRow, indexColumn, p_item );
		}

		p_item = new QTableWidgetItem;
		p_item->setText( QString::number( (int) (generalizor.m_vecRisk[vec_order[indexRow] ] * 100)) + "%" );
		ui.tableWidget_original->setItem( indexRow, ui.tableWidget_original->columnCount() - 1, p_item );

	}

	ui.tableWidget_original->resizeColumnsToContents();
	originalCurrentPage = 1;

	for(indexRow = 0; indexRow < upperBound; indexRow++){

		for(int indexColumn = 0; indexColumn < ui.tableWidget_generalized->columnCount(); indexColumn++){

			p_item = new QTableWidgetItem;
			p_item->setText( generalizor.GetValueQstr( vec_order[indexRow], indexColumn ) );
			ui.tableWidget_generalized->setItem( indexRow, indexColumn, p_item );
		}
	}

	ui.tableWidget_generalized->resizeColumnsToContents();
	generalizedCurrentPage = 1;

	// update risk histogram and utility graphs
	evaluateRisk( true, true );

	if ( ui.graphicsView_original->scene() != NULL && ui.graphicsView_generalized->scene() != NULL )
	{
		plotJointDensity();
	}
	if ( ui.tableWidget_contingency_generalized->rowCount() != 0 && ui.tableWidget_contingency_original->rowCount() != 0 )
	{
		createContingencyTable();
	}

	state = anony::AFTER_RECOVER;
	setState();
}

void myMainWindow::emphasizeTuple( int index )
{
	QMessageBox::information(this, "Message", QString::number( index ));

	UnitValue joint_x, joint_y, marginal;
	int index_joint_x, index_joint_y, index_marginal;
	if ( ui.tabWidget_data->currentWidget() == ui.tab_original )
	{
		int tupleIndex = index + ( originalCurrentPage - 1 ) * PAGE_SIZE;
		joint_x = data.GetValue( vec_order[tupleIndex], ui.comboBox_x_dimension_2->currentIndex());
		joint_y = data.GetValue( vec_order[tupleIndex], ui.comboBox_y_dimension_2->currentIndex());
		marginal = data.GetValue( vec_order[tupleIndex], ui.comboBox_dimension->currentIndex());
	}
	else if ( ui.tabWidget_data->currentWidget() == ui.tab_generalized )
	{
		int tupleIndex = index + ( generalizedCurrentPage - 1 ) * PAGE_SIZE;
		joint_x = data.GetValue( vec_order[tupleIndex], ui.comboBox_x_dimension_2->currentIndex());
		joint_y = data.GetValue( vec_order[tupleIndex], ui.comboBox_y_dimension_2->currentIndex());
		marginal = data.GetValue( vec_order[tupleIndex], ui.comboBox_dimension->currentIndex());
	}
	else
	{
		int tupleIndex = index + ( deletedCurrentPage - 1 ) * PAGE_SIZE;
		joint_x = data.GetValue( vec_del[tupleIndex], ui.comboBox_x_dimension_2->currentIndex());
		joint_y = data.GetValue( vec_del[tupleIndex], ui.comboBox_y_dimension_2->currentIndex());
		marginal = data.GetValue( vec_del[tupleIndex], ui.comboBox_dimension->currentIndex());
	}

	if ( data.GetVecMeta()[ui.comboBox_x_dimension_2->currentIndex()].m_nType == anony::INTEGER )
	{
		index_joint_x = joint_x.l - data.GetVecMeta()[ui.comboBox_x_dimension_2->currentIndex()].m_nMinValue.l;
	}
	else if ( data.GetVecMeta()[ui.comboBox_x_dimension_2->currentIndex()].m_nType == anony::CATEGORICAL )
	{
		index_joint_x = 0;
		for ( long i = 0;  i < joint_x.l; i++ )
		{
			if ( data.GetVecMeta()[ui.comboBox_x_dimension_2->currentIndex()].m_mapValueName.find( i ) != data.GetVecMeta()[ui.comboBox_x_dimension_2->currentIndex()].m_mapValueName.end() )
			{
				index_joint_x++;
			}
		}
	}
	else
	{
		index_joint_x = -1;
	}

	if ( data.GetVecMeta()[ui.comboBox_y_dimension_2->currentIndex()].m_nType == anony::INTEGER )
	{
		index_joint_y = joint_y.l - data.GetVecMeta()[ui.comboBox_y_dimension_2->currentIndex()].m_nMinValue.l;
	}
	else if ( data.GetVecMeta()[ui.comboBox_y_dimension_2->currentIndex()].m_nType == anony::CATEGORICAL )
	{
		index_joint_y = 0;
		for ( long i = 0;  i < joint_y.l; i++ )
		{
			if ( data.GetVecMeta()[ui.comboBox_y_dimension_2->currentIndex()].m_mapValueName.find( i ) != data.GetVecMeta()[ui.comboBox_y_dimension_2->currentIndex()].m_mapValueName.end() )
			{
				index_joint_y++;
			}
		}
	}
	else
	{
		index_joint_x = -1;
	}

	if ( data.GetVecMeta()[ui.comboBox_dimension->currentIndex()].m_nType == anony::INTEGER )
	{
		index_marginal = marginal.l - data.GetVecMeta()[ui.comboBox_dimension->currentIndex()].m_nMinValue.l;
	}
	else if ( data.GetVecMeta()[ui.comboBox_dimension->currentIndex()].m_nType == anony::CATEGORICAL )
	{
		index_marginal = 0;
		for ( long i = 0;  i < marginal.l; i++ )
		{
			if ( data.GetVecMeta()[ui.comboBox_dimension->currentIndex()].m_mapValueName.find( i ) != data.GetVecMeta()[ui.comboBox_dimension->currentIndex()].m_mapValueName.end() )
			{
				index_marginal++;
			}
		}
	}
	else
	{
		index_marginal = -1;
	}

	if ( index_joint_x > -1 && index_joint_y > -1 )
	{
	}

	if ( index_marginal > -1 )
	{
	}
}


*/

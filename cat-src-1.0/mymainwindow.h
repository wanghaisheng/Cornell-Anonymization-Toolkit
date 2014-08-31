#ifndef MYMAINWINDOW_H
#define MYMAINWINDOW_H

#include <QtGui>
#include <vector>
#include "ui_mymainwindow.h"

#include "openlog.h"
#include "openfile.h"
#include "openhierarchy.h"
#include "showmeta.h"
#include "showgen.h"
#include "aboutcat.h"

#include "MyLineEdit.h"

#include "Hierarchy.h"
#include "Microdata.h"
#include "Generalization.h"

#define PAGE_SIZE 60

#define BIN_NUM 50

#define BIN_WIDTH 8

#define BIN_HEIGHT 5

#define TOP_MARGIN 5

#define BOTTOM_MARGIN 15

#define LEFT_MARGIN 5

#define RIGHT_MARGIN 5

#define RISK_TEXT_SIZE 40

#define PERCENTAGE_TEXT_SIZE 60

#define X_AXIS_INTERVAL 10

#define Y_AXIS_INTERVAL 5

#define X_SUB_LEN 10

#define Y_SUB_LEN 16

#define NUMBER_LEN 20

#define MAX_WIDGTH 1000

#define MAX_HEIGHT 750

using namespace std;

class myLog
{

friend class myMainWindow;
friend class myOpenLog;

private:
 
	anony::LogType type_log; //	The type of the log (generalize, risk eval, delete)

	anony::VerifierType type_verifier; // The type of the verifier for generalization (diversity, closeness)

	int para_l; 	// The L parameter in Diversity for generalization
	double para_c;		// The C parameter in Diversity for generalization
	double para_t;		// The T parameter in Closeness for generalization

	vector<long> para_vec_del; 	// Vector of marked deleted tuples before generalization
	
	long para_num_del;		// Number of tuples to delete

	int para_piece_sa;		// Pieces of background knowledge about sensitive attribute in Diversity for risk evaluation
	vector<long> para_vec_bg_qi;	// Background knowledge about quasi-identifier for risk evaluation
	
	QTime timestamp;	// Timestamp of the log
};

class myMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	myMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~myMainWindow();

public slots:

	void setState();	// Set the system states (initialization, after-reading-data, after-reading-hierarchy, after-generalization, etc)

	void on_actionLoadData_triggered();			// Actions when load-data button is triggered
	void on_actionLoadHierarchy_triggered();	// Actions when load-hierarchy button is triggered
	void on_actionShowLog_triggered();			// Actions when show-log button is triggered
	void on_actionExit_triggered();				// Actions when exit button is triggered
	void on_actionPublish_triggered();			// Actions when publish button is triggered
	void on_actionMetaData_triggered();			// Actions when load-metadata button is triggered
	void on_actionGeneralization_triggered();	// Actions when generalization button is triggered
	void on_actionAbout_triggered();			// Actions when about button is triggered

	void readData(const QString&, const QString&);	// Read micro and meta data and render them to the original data table
	void readHierarchy(const QTableWidget *);		// Read hierarchy information
	void revert(const int);							// redo some executions in the log 

	void createContingencyTable();					// Create and compare two-dimensional contingency tables to illustrate the utility of the generalized data 
	void plotJointDensity();						// Plot and compare joint density figures to illustrate the utility of the generalized data
	void plotMarginalDensity();						// Plot and compare marginal density figures to illustrate the utility of the generalized data
	void plotMarginalDensityDifference();			// Plot the residuals of generalized marginal density figure to illustrate the utility of the generalized data
	void calcChiSquareTest();						// Calculate the Chi-Squared score of goodness-of-fit test

	void nextPage();								// Go to next page of the original / generalized / deleted data table
	void lastPage();								// Go to last page of the original / generalized / deleted data table
	void nextPageOriginal();						// Go to next page of the original data table
	void lastPageOriginal();						// Go to last page of the original data table
	void nextPageGeneralized();						// Go to next page of the generalized data table
	void lastPageGeneralized();						// Go to last page of the generalized data table
	void nextPageDeleted();							// Go to next page of the deleted data table
	void lastPageDeleted();							// Go to last page of the deleted data table

	void generalize( bool redo = false );			// Generalize the data according to some verifier type (L-Diversity, T-Closeness, etc)
	void evaluateRisk( bool redo = false, bool needsUpdate = true );	// Evaluate Risk of the generalized data according to some verifier type (L-Diversity, T-Closeness, etc)
	void rescaleRisk();								// Rescale the risk showing granularity
	void genHisto( const vector<long>& vecCnt, float maxRisk, float minRisk, QGraphicsScene& scnHisto );	// Generate risk histograms after it is evaluated
	void changeThreshold(double threshold);			// Change the threshold of deletion when the slider is dragged or the combox is set

	void setInt2Double_c(int);						// Transform integer in combobox to double in slider for parameter c
	void setDouble2Int_c(double);					// Transform double in slider to integer in combobox for parameter c
	void setInt2Double_t(int);						// Transform integer in combobox to double in slider for parameter t
	void setDouble2Int_t(double);					// Transform double in slider to integer in combobox for parameter t
	void setInt2Double_r(int);						// Transform integer in combobox to double in slider for rescaling factor
	void setDouble2Int_r(double);					// Transform double in slider to integer in combobox for rescaling factor
	void setInt2Double_th(int);						// Transform integer in combobox to double in slider for deleting threshold
	void setDouble2Int_th(double);					// Transform double in slider to integer in combobox for deleting threshold

	void deleteTuple( bool redo  = false );			// Delete tuples according to the deleting threshold
	void numTuple( double threshold = 0 );			// Calculate number of tuples to be deleted
	void updateDeleted();							// Update the data tables after deletion

	void sortByAttr( int attrIndex = -1 );			// Sort tuples in data tables according to the attribute

//	void emphasizeTuple(int);						// Emphasize the selected tuple in the density figure

	void setOriginalCTHorizontal( int );			// Synchronize horizontal sliders for original table according to generalized contingency table
	void setOriginalCTVertical( int );				// Synchronize vertical sliders for original table according to generalized contingency table
	void setGeneralizedCTHorizontal( int );			// Synchronize horizontal sliders for generalized table according to original contingency table
	void setGeneralizedCTVertical( int );			// Synchronize vertical sliders for generalized original table according to original contingency table

	void setCurrentPage( int );						// Disable and Enable "Last Page" and "Next Page" buttons according to the current data table
	void setCurrentVerifier( int );					// Disable and Enable Special background knowledge settings according to the current verifier

public:
	Microdata data;		// The object storing the input micro and meta data
	Generalization generalizor;		// The object storing the generalization information

private:

	Ui::myMainWindowClass ui;

	myOpenFile *openFile;				// Sub-window for opening file
	myOpenHierarchy *openHierarchy;		// Sub-window for opening hierarchy
	myOpenLog *openLog;					// Sub-window for opening log
	myShowMeta *showMeta;				// Sub-window for showing meta information
	myShowGeneralization *showGen;		// Sub-window for showing generalization information 
	myAboutCAT *aboutCAT;				// Sub-window for showing CAT information

	QGraphicsLineItem *thresHold;		// Threshold red-bar on the risk histogram views
	
	vector<long> vec_qi;				// Vector for storing quasi-identifier indices
	vector<string> vec_hrch;			// Vector for storing hierarchy file names
	vector<long> vec_del;				// Vector for storing deleted tuple indices
	vector<long> vec_order;				// Vector for storing tuple ordering indices
	vector<class myLog> vec_log;		// Vector for storing user logs

	int sensAttr;						// Sensitive attribute index
	float minRisk;						// Minimum risk boundary for rescaling
	float maxRisk;						// Maximum risk boundary for rescaling
	
	int originalCurrentPage;			// Current page of original data table
	int generalizedCurrentPage;			// Current page of generalized data table
	int deletedCurrentPage;				// Current page of deleted data table
	int maxPage;						// Total pages of original and generalized data table
	int maxDeletePage;					// Total pages of deleted data table

	bool ascend;						// Order of sorting (ascend, descend)
	int sortIndex;						// Index of attribute for sorting

	anony::StateType state;				// Recording current system state (initialization, after-reading-data, after-reading-hierarchy, after-generalization, etc)
};

#endif // MYMAINWINDOW_H

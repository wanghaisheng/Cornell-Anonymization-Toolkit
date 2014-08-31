#ifndef SHOWMETA_H
#define SHOWMETA_H

#include <QtGui>
#include "ui_showmeta.h"
#include "Generalization.h"

class myShowMeta : public QWidget
{
	Q_OBJECT

friend class myMainWindow;

public:
	myShowMeta(QWidget *parent = 0);
	~myShowMeta()
	{
		this->close();
	}


public slots:
	void okay();	// Close this sub-window
	void loadMeta( Generalization& meta );		// Load meta data information from main window
	void NumLevels( int index );				// Show number of level for certain attribute by index
	void MetaInfo( int index );					// Show meta information for certain attribute by index
	void NumValues( int index );				// Show number of values for certain hierarchy level by index
	void ValueInfo( int index );				// Show value information for certain hierarchy level by index
	void ChildrenCoverage( int index );			// Darken children nodes of certain value by index

private:
	Ui::myOpenmetaClass ui;
	Generalization	*meta_info;		// Meta data information of attributes
	vector<long> vec_leaves;		// Vector storing the leaves of the hierarchy tree
	int index_meta;					// Attribute meta information index
	int index_level;				// Attribute current hierarchy level index
	bool qi;						// Qi dummy (true if it is Qi attribute)

};

#endif
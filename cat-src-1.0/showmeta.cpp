#include "showmeta.h"

// including <QtGui> saves us to include every class user, <QString>, <QFileDialog>,...

myShowMeta::myShowMeta(QWidget *parent)
{
	ui.setupUi(this); // this sets up GUI

	ui.lineEdit_name->setReadOnly( true );
	ui.lineEdit_type->setReadOnly( true );
	ui.lineEdit_level->setReadOnly( true );
	ui.lineEdit_value->setReadOnly( true );
	ui.lineEdit_missing->setReadOnly( true );
	ui.radioButton_qi->setAutoExclusive( false );
	ui.radioButton_sa->setAutoExclusive( false );
	ui.listWidget_children->setSelectionMode( QAbstractItemView::MultiSelection );
	this->setWindowModality( Qt::ApplicationModal );

	// signals/slots mechanism in action
	connect( ui.pushButton_ok, SIGNAL( clicked() ), this, SLOT( okay() ) );
	connect( ui.comboBox_attribute, SIGNAL( currentIndexChanged ( int ) ), this, SLOT( MetaInfo( int ) ) );
	connect( ui.comboBox_attribute, SIGNAL( currentIndexChanged ( int ) ), this, SLOT( NumLevels( int ) ) );
	connect( ui.comboBox_level, SIGNAL( currentIndexChanged ( int ) ), this, SLOT( NumValues( int ) ) );
	connect( ui.comboBox_level, SIGNAL( currentIndexChanged ( int ) ), this, SLOT( ValueInfo( int ) ) );
	connect( ui.comboBox_value, SIGNAL( currentIndexChanged ( int ) ), this, SLOT( ChildrenCoverage( int ) ) );

	index_level = -1;
	index_meta = -1;
	qi = false;
}

void myShowMeta::okay()
{
	this->close();
}

void myShowMeta::loadMeta( Generalization& meta )
{
	meta_info = &meta;

	QStringList temp_list;
	for( int index = 0; index < meta_info->m_pMicrodata->GetAttrNum(); index++)
	{
		QString qstr( meta_info->m_pMicrodata->GetVecMeta()[index].m_strName.c_str() );
		temp_list << qstr;
	}
	ui.comboBox_attribute->clear();
	ui.comboBox_attribute->addItems( temp_list );
}

void myShowMeta::MetaInfo( int index )
{
	if (index < 0)
	{
		return;
	}

	index_meta = index;
	
	ui.lineEdit_name->setText( meta_info->m_pMicrodata->GetVecMeta()[index].m_strName.c_str() );

	if ( meta_info->m_pMicrodata->GetVecMeta()[index].m_nType == anony::CATEGORICAL )
	{
		ui.lineEdit_type->setText( "CATEGORICAL" );
	}
	else if ( meta_info->m_pMicrodata->GetVecMeta()[index].m_nType == anony::INTEGER )
	{
		ui.lineEdit_type->setText( "INTEGER" );
	}
	else if ( meta_info->m_pMicrodata->GetVecMeta()[index].m_nType == anony::FLOAT )
	{
		ui.lineEdit_type->setText( "FLOAT" );
	}

	ui.lineEdit_missing->setText( QString::number( meta_info->m_pMicrodata->GetVecMeta()[index].m_vecMissValue.size() ) );

	
}

void myShowMeta::NumLevels( int index )
{
	if (index < 0)
	{
		return;
	}

	bool QiFlag = false;
	for ( int QiIndex = 0; QiIndex < meta_info->m_vecQiAttr.size(); QiIndex++)
	{
		if ( meta_info->m_vecQiAttr[QiIndex].m_nAttr == index )
		{
			QiFlag = true;
			break;
		}

	}
	if ( QiFlag )
	{
		qi = true;

		ui.radioButton_qi->setChecked( true );
		ui.radioButton_sa->setChecked( false );

		ui.lineEdit_level->setText( QString::number( meta_info->m_vecHrch[meta_info->m_vecQiInd[index]].GetVecLevel().size() + 1 ) );

		QStringList temp_list;
		for( int levelIndex = 0; levelIndex <= meta_info->m_vecHrch[meta_info->m_vecQiInd[index]].GetVecLevel().size(); levelIndex++)
		{
			QString qstr( "Level " + QString::number( levelIndex ) );
			temp_list << qstr;
		}
		ui.comboBox_level->clear();
		ui.comboBox_level->addItems ( temp_list );
	}
	else if ( meta_info->m_nSenAttr == index )
	{
		qi = false;

		ui.radioButton_sa->setChecked( true );
		ui.radioButton_qi->setChecked( false );

		ui.lineEdit_level->setText( "1" );
		ui.comboBox_level->clear();
		ui.comboBox_level->addItem( "Level 0" );
	}
	else
	{
		qi = false;

		ui.radioButton_qi->setChecked( false );
		ui.radioButton_sa->setChecked( false );

		ui.lineEdit_level->setText( "1" );
		ui.comboBox_level->clear();
		ui.comboBox_level->addItem( "Level 0" );
	}
}

void myShowMeta::NumValues( int index)
{
	if ( index < 0 )
	{
		return;
	}

	if ( qi && index > 0 )
	{
		ui.lineEdit_value->setText( QString::number( meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetCount( index - 1 ) ) );
	}
	else
	{
		switch ( meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_nType )
		{
		case anony::INTEGER :
			{
				ui.lineEdit_value->setText( QString::number( meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_nMinValue.l ) +  " - " + QString::number( meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_nMaxValue.l ) );
				break;
			}
		case anony::CATEGORICAL :
			{
				ui.lineEdit_value->setText( QString::number( meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_mapValueName.size() ) );
				break;
			}
		case anony::FLOAT :
			{
				ui.lineEdit_value->setText( QString::number( meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_nMinValue.f ) +  " - " + QString::number( meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_nMaxValue.f ) );
				break;
			}

		}
	}
}

void myShowMeta::ValueInfo( int index )
{
	if ( index < 0 )
	{
		return;
	}

	index_level = index;

	ui.listWidget_children->clear();
	vec_leaves.clear();

	if ( qi && index > 0 )
	{
		ui.comboBox_value->setDisabled( false );

		QStringList temp_list;
		for( int valueIndex = 0; valueIndex < meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetCount( index - 1 ); valueIndex++)
		{
			QString qstr( meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetNode(valueIndex + meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetVecLevel()[ index - 1 ]).m_strName.c_str() );
			temp_list << qstr;
		}
		ui.comboBox_value->clear();
		ui.comboBox_value->addItems ( temp_list );

		if ( index == 1)
		{
			switch ( meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_nType )
			{
			case anony::INTEGER :
				{
					break;
				}
			case anony::CATEGORICAL :
				{
					for ( int valueIndex = meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_nMinValue.l; valueIndex <= meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_nMaxValue.l; valueIndex++ )
					{
						if ( meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_mapValueName.find( valueIndex ) != meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_mapValueName.end() )
						{
							vec_leaves.push_back( valueIndex );
						}
					}

					QStringList temp_list;
					for( long valueIndex = 0; valueIndex < vec_leaves.size(); valueIndex++)
					{
						QString qstr( (*(meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_mapValueName.find( vec_leaves[valueIndex] ))).second.c_str() );
						temp_list << qstr;
					}
					ui.listWidget_children->addItems ( temp_list );

					ChildrenCoverage( 0 );

					break;
				}
			case anony::FLOAT :
				{
					break;
				}

			}
		}
		else
		{
			QStringList temp_list;
			for( int valueIndex = 0; valueIndex < meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetCount( index - 2 ); valueIndex++)
			{
				QString qstr( meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetNode(valueIndex + meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetVecLevel()[ index - 2 ]).m_strName.c_str() );
				temp_list << qstr;
			}
			ui.listWidget_children->addItems( temp_list );

			ChildrenCoverage( 0 );
		}
	}
	else
	{
		ui.comboBox_value->clear();
		ui.comboBox_value->setDisabled( true );
	}
}

void myShowMeta::ChildrenCoverage( int index )
{
	if ( index < 0 )
	{
		return;
	}
	if ( index_level < 1 )
	{
		return;
	}
	if ( index_level < 2 && meta_info->m_pMicrodata->GetVecMeta()[index_meta].m_nType != anony::CATEGORICAL )
	{
		return;
	}
	if ( ui.listWidget_children->count() == 0 )
	{
		return;
	}

	ui.listWidget_children->clearSelection();
	
	if ( index_level == 1)
	{
		long index_left = meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetNode(index + meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetVecLevel()[ index_level - 1 ]).m_uInterval.m_uLeft.l;
		long index_right = meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetNode(index + meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetVecLevel()[ index_level - 1 ]).m_uInterval.m_uRight.l;

		int index_child = 0;
		while ( vec_leaves[index_child] < index_left )
			index_child++;

		while ( index_child < vec_leaves.size() && vec_leaves[index_child] <= index_right )
		{
			ui.listWidget_children->item( index_child )->setSelected( true );
			index_child++;
		}
	}
	else
	{
		long index_left = meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetNode(index + meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetVecLevel()[ index_level - 1 ]).m_nChildLeft - meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetVecLevel()[ index_level - 2 ];
		long index_right = meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetNode(index + meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetVecLevel()[ index_level - 1 ]).m_nChildRight - meta_info->m_vecHrch[meta_info->m_vecQiInd[index_meta]].GetVecLevel()[ index_level - 2 ];

		for ( int index_child = index_left; index_child <= index_right; index_child++ )
		{
			ui.listWidget_children->item( index_child )->setSelected( true );
		}


	}

}
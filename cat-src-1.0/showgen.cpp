#include "showgen.h"

// including <QtGui> saves us to include every class user, <QString>, <QFileDialog>,...

myShowGeneralization::myShowGeneralization(QWidget *parent)
{
	ui.setupUi(this); // this sets up GUI

	ui.lineEdit_name->setReadOnly( true );
	ui.lineEdit_level->setReadOnly( true );
	ui.lineEdit_weight->setReadOnly( true );
	this->setWindowModality( Qt::ApplicationModal );

	// signals/slots mechanism in action
	connect( ui.pushButton_ok, SIGNAL( clicked() ), this, SLOT( okay() ) );
	connect( ui.comboBox_qi, SIGNAL( currentIndexChanged ( int ) ), this, SLOT( GeneralizationInfo( int ) ) );
}

void myShowGeneralization::okay()
{
	this->close();
}

void myShowGeneralization::loadGen( Generalization& gen )
{
	gen_info = &gen;

	QStringList temp_list;
	for( int index = 0; index < gen_info->m_vecQiAttr.size(); index++)
	{
		QString qstr( gen_info->m_pMicrodata->GetVecMeta()[gen_info->m_vecQiAttr[index].m_nAttr].m_strName.c_str() );
		temp_list << qstr;
	}
	ui.comboBox_qi->clear();
	ui.comboBox_qi->addItems( temp_list );
}

void myShowGeneralization::GeneralizationInfo( int index )
{
	if ( index < 0 )
	{
		return;
	}

	ui.lineEdit_name->setText( gen_info->m_pMicrodata->GetVecMeta()[gen_info->m_vecQiAttr[index].m_nAttr].m_strName.c_str() );
	ui.lineEdit_level->setText( QString::number( gen_info->m_vecGenLevel[gen_info->m_vecQiAttr[index].m_nAttr] ) );
	ui.lineEdit_weight->setText( QString::number( gen_info->m_vecQiAttr[index].m_fWght ) );
}

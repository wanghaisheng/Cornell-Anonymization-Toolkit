// Anonymizer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Hierarchy.h"
#include "Microdata.h"
#include "Generalization.h"
#include <iostream>
#include <fstream>
#include <algorithm>


int _tmain(int argc, _TCHAR* argv[])
{
	//	Hierarchy test_hrch;
	//	test_hrch.Init( "./Hierarchies/Race.txt", 1000 );

	Microdata test_micro;
	test_micro.ReadFiles( "./Data/Adult-Sal.txt", "./Data/Adult-Sal-Meta.txt" );

	long i, j, k;
	const vector<UnitValue>& r_micro = test_micro.GetVecTuple();
	const vector<Metadata>& r_meta = test_micro.GetVecMeta();
	long attr_num = r_meta.size();
	long tuple_num = r_micro.size() / attr_num;
	// 	for (i = 0; i < 10; ++i)
	// 	{
	// 		cout << "Tuple " << i + 1 << ":" << endl;
	// 		for (j = 0; j < attr_num; ++j)
	// 		{
	// 			cout << r_meta[j].m_strName << ": ";
	// //			anony::AttrType temp_type = r_meta[j].m_nType;
	// 			if (r_meta[j].m_nType == anony::INTEGER)
	// 			{
	// 				cout << r_micro[i * attr_num + j].l << endl;
	// 			}
	// 			else if (r_meta[j].m_nType == anony::CATEGORICAL)
	// 			{
	// 				cout << r_micro[i * attr_num + j].l << " - ";
	// 				const pair<long, string>& r_pair = *r_meta[j].m_mapValueName.find( r_micro[i * attr_num + j].l );
	// 				cout << r_pair.second << endl;
	// 			}
	// 			else
	// 			{
	// 				cout << r_micro[i * attr_num + j].f << endl;
	// 			}
	// 		}
	// 		cout << endl;
	// 	}



	vector<long> vec_qi;
		vec_qi.push_back( 0 );
	//	vec_qi.push_back( 1 );
	//	vec_qi.push_back( 2 );
		vec_qi.push_back( 3 );
	vec_qi.push_back( 4 );
	//	vec_qi.push_back( 5 );

	vector<string> vec_hrch;
	vec_hrch.push_back( "./Hierarchies/Age.txt" );
	//	vec_hrch.push_back( "./Hierarchies/Gender.txt" );
	//	vec_hrch.push_back( "./Hierarchies/Race.txt" );
	vec_hrch.push_back( "./Hierarchies/Marital Status.txt" );
	vec_hrch.push_back( "./Hierarchies/Birth Place.txt" );
	//	vec_hrch.push_back( "./Hierarchies/Education.txt" );
	

	Generalization test_gen;
	test_gen.Init( &test_micro, vec_hrch, vec_qi, 8 );
	cout << "Initialization finished" << endl;


	L_Diversity temp_verifier;
	temp_verifier.m_fC = 2;
	temp_verifier.m_nL = 2;

	try
	{
		temp_verifier.CheckBeforeAnonymize( test_gen );
	}
	catch( AnonyException e )
	{
		getchar();
	}

	test_gen.Anonymize( temp_verifier );


	temp_verifier.m_nBckKnw = 2;

	try
	{
		temp_verifier.CheckBeforeEvalRisk( test_gen );
	}
	catch( AnonyException e )
	{
		getchar();
	}

	test_gen.EvaluateRisk( vec_qi, temp_verifier );

	/*
	list<EqvClass>::iterator eqv_pos;
	long eqv_cnt = 0;
	for (eqv_pos = test_gen.m_lstEqvClass.begin(); eqv_pos != test_gen.m_lstEqvClass.end(); ++eqv_pos)
	{
	const EqvClass& r_eqv = *eqv_pos;
	long tpl_no = *r_eqv.m_lstTplId.begin();
	cout << "Equivalence Class " << eqv_cnt << ":" << endl;

	for (i = 0; i < 3; ++i)
	{
	if (r_eqv.m_vecNodePos[i] == -1) 
	{				
	cout << r_micro[tpl_no * attr_num + i].l << "; ";
	}
	else
	{
	const vector<HrchNode>& r_vec_node = test_gen.m_vecHrch[i].GetVecNode();
	cout << r_vec_node[r_eqv.m_vecNodePos[i]].m_uInterval.m_uLeft.l << "-" << r_vec_node[r_eqv.m_vecNodePos[i]].m_uInterval.m_uRight.l << "; ";
	}
	}

	cout << endl;
	++eqv_cnt;
	}

	//	vec_qi.resize( 2 );
	test_gen.EvaluateRisk( vec_qi, 0 );
	ofstream log_file;
	log_file.open( "test_log.txt" );
	for (i = 0; i < (test_gen.m_pMicrodata->GetVecTuple()).size() / (test_gen.m_pMicrodata->GetVecMeta()).size(); ++i)
	{
	log_file << test_gen.m_vecRisk[i] << endl;
	}
	log_file.close();
	*/

/*
	vector<float> vec_ori, vec_anony;
	vector<QString> vec_name_x, vec_name_y;
	long height, width;
	test_gen.GetDensity( 0, 4, height, width, vec_ori, vec_anony, vec_name_x, vec_name_y );

	for (i = 0; i < height * width; ++i)
	{
		cout << vec_ori[i] << " " << vec_anony[i] << ";  ";
		if (i % 10 == 9)
		{
		cout << endl;
		}
		if (i % 50 == 49)
		{
		getchar();
		}
	}
*/


	getchar();

	return 0;
}


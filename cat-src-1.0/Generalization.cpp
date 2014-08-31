#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "Generalization.h"
#include "AnonyException.h"


//////////////////////////////////////////////////////////////////////////


bool Generalization::Init( Microdata* pMicrodata, const vector<string>& rVecHrchFile, const vector<long>& rVecQi, const long nSenAttr )
{
	AnonyException temp_exception;
	temp_exception.m_bShallContinue = true;
	stringstream temp_sstream;

	Clear();

	try
	{
		if (pMicrodata == NULL)
		{
			temp_sstream << "Generalization::Init Error: Pointer to the microdata cannot be NULL";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}

		m_pMicrodata = pMicrodata;

		m_vecStatus.insert( m_vecStatus.end(), m_pMicrodata->GetTupleNum(), anony::NORMAL );

		const vector<Metadata>& r_meta = pMicrodata->GetVecMeta();

		if (rVecHrchFile.size() != rVecQi.size() || rVecQi.size() == 0 || rVecQi.size() > r_meta.size())
		{
			temp_sstream << "Generalization::Init Error: Incorrect number of hierarchy files or QI attributes";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}

		if (nSenAttr < 0 || nSenAttr >= r_meta.size() || find( rVecQi.begin(), rVecQi.end(), nSenAttr ) != rVecQi.end() )
		{
			temp_sstream << "Generalization::Init Error: Incorrect sensitive attribute";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}
		m_nSenAttr = nSenAttr;
		

		long i, j, k;
		const long qi_num = rVecQi.size();

		//	Initialize m_vecQiAttr
		AttrWghtPair temp_pair;
		temp_pair.m_fWght = 1;
		m_vecQiAttr.reserve( qi_num );			
		for (i = 0; i < qi_num; ++i)
		{
			temp_pair.m_nAttr = rVecQi[i];
			m_vecQiAttr.push_back( temp_pair );
		}

		//	Check whether the QI attributes specified in rVecQI are correct
		sort( m_vecQiAttr.begin(), m_vecQiAttr.end(), CompareByAttr() );

		if ((m_vecQiAttr.back().m_nAttr < 0) || (m_vecQiAttr.back().m_nAttr >= m_pMicrodata->GetAttrNum()))
		{
			temp_sstream << "Generalization::Init Error: Incorrect QI attribute indices";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}

		for (i = 1; i < qi_num; ++i)
		{
			if (m_vecQiAttr[i].m_nAttr == m_vecQiAttr[i - 1].m_nAttr)
			{
				temp_sstream << "Generalization::Init Error: There exist duplicate QI attributes";
				temp_exception.SetMsg( temp_sstream.str() );
				throw( temp_exception );
			}
		}


		//	Initialize m_vecQiInd
		m_vecQiInd.insert( m_vecQiInd.begin(), m_pMicrodata->GetAttrNum(), -1 );
		for (i = 0; i < qi_num; ++i)
		{
			m_vecQiInd[m_vecQiAttr[i].m_nAttr] = m_vecQiAttr[i].m_nAttr;
		}

		m_vecHrch.insert( m_vecHrch.end(), r_meta.size(), Hierarchy() );
		for (i = 0; i < qi_num; ++i)
		{
			m_vecHrch[m_vecQiAttr[i].m_nAttr].Init( rVecHrchFile[i].c_str(), 100000, r_meta[m_vecQiAttr[i].m_nAttr].m_nType );
		}

		//	Initialize the "root" equivalence class
		InitRootClass();
	}
	catch( AnonyException e )
	{
		Clear();
		cout << e.GetMsg() << endl;
		throw( e );
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////


bool Generalization::InitRootClass()
{
	try
	{
		const vector<Metadata>& r_meta = m_pMicrodata->GetVecMeta();
		const vector<UnitValue>& r_micro = m_pMicrodata->GetVecTuple();
		const long attr_num = r_meta.size();
		const long tuple_num = r_micro.size() / attr_num;
		const long qi_num = m_vecQiAttr.size();

		long i, j, k;

		m_eqvRootClass.Clear();

		long cur_attr;
		map<UnitValue, long>::iterator map_pos;

		//	Insert all eligible tuples into the "root" equivalence class
		m_vecCantGen.clear();
		for (i = 0; i < tuple_num; ++i)
		{
			if (m_vecStatus[i] == anony::DELETED)	
			{
				continue;	//	If a tuple is deleted, ignore it
			}
			else
			{
				m_vecStatus[i] = anony::NORMAL;
			}


			for (j = 0; j < qi_num; ++j)
			{
				cur_attr = m_vecQiAttr[j].m_nAttr;
				const vector<UnitValue>& r_missing = r_meta[cur_attr].m_vecMissValue;
				if (find( r_missing.begin(), r_missing.end(), r_micro[i * attr_num + cur_attr] ) != r_missing.end())	
				{
					m_vecStatus[i] = anony::CANTGEN;	//	If a tuple has a missing value on some QI attribute, ignore it
					break;
				}
				const vector<HrchNode>& r_vec_nodes = m_vecHrch[m_vecQiAttr[j].m_nAttr].GetVecNode();	
				if ((r_vec_nodes.back()).m_uInterval.Covers( r_micro[i * attr_num + cur_attr] ) == false)
				{
					m_vecStatus[i] = anony::CANTGEN;	//	If a tuple has a QI value that is not in the generalization hierarchy, ignore it
					break;
				}
			}

			if (m_vecStatus[i] == anony::CANTGEN)	
			{
				m_vecCantGen.push_back( i );	//	Put into m_vecCantGen the IDs of those tuples that cannot be generalized
			}
			else
			{
				//	Put the eligible tuples into the "root" equivalence class
				m_eqvRootClass.m_lstTplId.push_back( i );

				//	Update the frequencies of the sensitive values
				map<UnitValue, long>& r_map = m_eqvRootClass.m_mapSenCnt;
				map_pos = r_map.find( r_micro[i * attr_num + m_nSenAttr] );
				if (map_pos == r_map.end())
				{
					r_map.insert( pair<UnitValue, long>( r_micro[i * attr_num + m_nSenAttr], 1 ) );
				}
				else
				{
					++(*map_pos).second;
				}
			}
		}

		//	Initialize m_eqvRootClass.m_vecNodePos
		m_eqvRootClass.m_vecNodePos.insert( m_eqvRootClass.m_vecNodePos.end(), attr_num, -1 );
		for (j = 0; j < qi_num; ++j)
		{
			const vector<HrchNode>& r_vec_nodes = m_vecHrch[m_vecQiAttr[j].m_nAttr].GetVecNode();
			m_eqvRootClass.m_vecNodePos[m_vecQiAttr[j].m_nAttr] = r_vec_nodes.size() - 1;
		}
	}
	catch( AnonyException e )
	{
		Clear();
		cout << e.GetMsg() << endl;
		throw( e );
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////


bool Generalization::SetWeights( const vector<float>& rVecWeights )
{
	AnonyException temp_exception;
	temp_exception.m_bShallContinue = true;
	stringstream temp_sstream;

	try
	{
		if (rVecWeights.size() != m_vecQiAttr.size())
		{
			temp_sstream << "Generalization::SetWeithgt Error: The number of weights differs from the number of QI attributes";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}

		if (*max_element( rVecWeights.begin(), rVecWeights.end() ) > 1 || *min_element( rVecWeights.begin(), rVecWeights.end() ) < 1)
		{
			temp_sstream << "Generalization::SetWeithgt Error: Weights cannot be larger than 100 or smaller than 1";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}

		long i;
		const long temp_size = rVecWeights.size();
		for (i = 0; i < temp_size; ++i)
		{
			m_vecQiAttr[i].m_fWght = rVecWeights[i];
		}
	}
	catch( AnonyException e )
	{
		Clear();
		cout << e.GetMsg() << endl;
		throw( e );
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////


bool Generalization::Anonymize( Verifier& rVerifier )
{
	try
	{
		AnonyException temp_exception;
		temp_exception.m_bShallContinue = true;
		stringstream temp_sstream;

		if (m_pMicrodata == NULL)
		{
			temp_sstream << "Generalization::Anonymize Error: Pointer to the microdata is NULL";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );		
		}

		if (m_eqvRootClass.m_lstTplId.size() == 0)
		{
			temp_sstream << "Generalization::Anonymize Error: Root equivalence class has not been initialized";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );			
		}

		//	Check whether the tuples in the "root" equivalence class need to be reloaded or not
		if (m_bRootDirty == true)
		{
			InitRootClass();
			m_bRootDirty = false;
		}

		long i, j, k;
		long temp_cnt;
		temp_cnt = 0;

		//	If the "root" equivalence class does not satisfy the privacy criterion, stop here
		if (rVerifier.Verify( m_eqvRootClass ) == false)
		{
			temp_sstream << "Generalization::Anonymize Error: No generalization satisfies the given requirement";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}

		const vector<Metadata>& r_meta = m_pMicrodata->GetVecMeta();
		const vector<UnitValue>& r_micro = m_pMicrodata->GetVecTuple();
		const long attr_num = r_meta.size();
		const long tuple_num = r_micro.size() / attr_num;
		const long qi_num = m_vecQiAttr.size();

		//	Initialize m_vecGenLevel
		m_vecGenLevel.clear();
		m_vecGenLevel.insert( m_vecGenLevel.end(), attr_num, -1 );
		for (i = 0; i < qi_num; ++i)
		{
			const vector<long>& r_vec_level = m_vecHrch[m_vecQiAttr[i].m_nAttr].GetVecLevel();
			m_vecGenLevel[m_vecQiAttr[i].m_nAttr] = r_vec_level.size();
		}

		//	Starts from the coarsest generalization, i.e., a generalization based on the "root" equivalence class
		m_lstEqvClass.clear();
		m_lstEqvClass.push_back( m_eqvRootClass );

		list<EqvClass> temp_lst_eqvcls;
		list<EqvClass> done_lst_eqvcls;
		vector<AttrWghtPair> vec_attr_order = m_vecQiAttr;
		map<UnitValue, list<EqvClass>::iterator> map_eqvcls;
		map<UnitValue, list<EqvClass>::iterator>::iterator map_pos;
		pair<long, list<EqvClass>::iterator> temp_pair;
		long failure_cnt = 0;
		bool gen_failed;
		HrchNode temp_node;

		//	Iteratively refine the generalization
		while (failure_cnt < qi_num)
		{
			//	Sort the attributes according to their weights
			long cur_attr;
			sort( vec_attr_order.begin(), vec_attr_order.end(), CompareByWght() );

			//	Try to refine the attribute with the highest weight; if fail, try the attribute with the second highest weight, and so on
			for (i = 0; i < qi_num; ++i)
			{				
				cur_attr = vec_attr_order[i].m_nAttr;
				if (m_vecGenLevel[cur_attr] == 0)
				{
					++failure_cnt;	//	If we have reached the leaf level of attribute cur_attr, we are done with it
					vec_attr_order[i].m_fWght = 0;
					continue;
				}

				//	Try to refine attribute cur_attr
				list<EqvClass>::iterator cls_pos;
				gen_failed = false;
				for (cls_pos = m_lstEqvClass.begin(); cls_pos != m_lstEqvClass.end(); ++cls_pos)	//	Check each equivalence class
				{
					//	Refine the equivalence class along attribute cur_attr
					map_eqvcls.clear();
					const EqvClass& r_eqvcls = *cls_pos;
					const vector<HrchNode> r_vec_hrch = m_vecHrch[cur_attr].GetVecNode();
					long node_begin, node_end;
					if (r_eqvcls.m_vecNodePos[cur_attr] < 0)
					{
						node_begin = node_end = -1;
					}
					else
					{						
						node_begin = r_vec_hrch[r_eqvcls.m_vecNodePos[cur_attr]].m_nChildLeft;
						node_end = r_vec_hrch[r_eqvcls.m_vecNodePos[cur_attr]].m_nChildRight;
					}
					list<long>::const_iterator tpl_pos;
					vector<HrchNode>::const_iterator node_pos;
					map<UnitValue, long>::iterator sen_pos;
					UnitValue eqvcls_key;
					for (tpl_pos = r_eqvcls.m_lstTplId.begin(); tpl_pos != r_eqvcls.m_lstTplId.end(); ++tpl_pos)
					{
						long tpl_no = *tpl_pos;
						if (node_begin == -1)
						{
							eqvcls_key = r_micro[tpl_no * attr_num + cur_attr];
						}
						else
						{
							temp_node.m_uInterval.m_uLeft = temp_node.m_uInterval.m_uRight = r_micro[tpl_no * attr_num + cur_attr];
							node_pos = lower_bound( r_vec_hrch.begin() + node_begin, r_vec_hrch.begin() + node_end + 1, temp_node );
							eqvcls_key = long( distance( r_vec_hrch.begin(), node_pos ) );

							if (eqvcls_key.l == node_end + 1)
							{
								temp_sstream << "Generalization::Anonymize Error: Value " << cur_attr << " of tuple " << tpl_no << " is not covered in level " << m_vecGenLevel[cur_attr] << " of the hierarchy";
								temp_exception.SetMsg( temp_sstream.str() );
								throw( temp_exception );
							}
						}
						map_pos = map_eqvcls.find( eqvcls_key );
						if (map_pos != map_eqvcls.end())
						{
							EqvClass& r_target = *((*map_pos).second);
							r_target.m_lstTplId.push_back( tpl_no );
							map<UnitValue, long>& r_sen_map = r_target.m_mapSenCnt;
							sen_pos = r_sen_map.find( r_micro[tpl_no * attr_num + m_nSenAttr] );
							if (sen_pos == r_sen_map.end())
							{
								r_sen_map.insert( pair<UnitValue, long>( r_micro[tpl_no * attr_num + m_nSenAttr], 1 ) );
							}
							else
							{
								++(*sen_pos).second;
							}
						}
						else
						{
							temp_lst_eqvcls.push_front( EqvClass() );
							EqvClass& r_target = *temp_lst_eqvcls.begin();
							r_target.m_lstTplId.push_back( tpl_no );
							r_target.m_mapSenCnt.insert( pair<UnitValue, long>( r_micro[tpl_no * attr_num + m_nSenAttr], 1 ) );
							r_target.m_vecNodePos = r_eqvcls.m_vecNodePos;
							if (node_begin == -1)
							{
								r_target.m_vecNodePos[cur_attr] = -1;
							}
							else
							{
								r_target.m_vecNodePos[cur_attr] = eqvcls_key.l;
							}							
							map_eqvcls.insert( pair<UnitValue, list<EqvClass>::iterator>( eqvcls_key, temp_lst_eqvcls.begin() ) );
						}
					}

					list<EqvClass>::iterator temp_cls_pos;
					for (temp_cls_pos = temp_lst_eqvcls.begin(); temp_cls_pos != temp_lst_eqvcls.end(); ++temp_cls_pos)
					{
						EqvClass& r_test = *temp_cls_pos;

						if (rVerifier.Verify( r_test ) == false)
						{
							gen_failed = true;
							break;
						}
					}

					if (gen_failed == true)
					{
						temp_lst_eqvcls.clear();
						done_lst_eqvcls.clear();
						break;
					}

					done_lst_eqvcls.splice( done_lst_eqvcls.end(), temp_lst_eqvcls );
				}

				if (gen_failed == true)
				{
					++failure_cnt;
					vec_attr_order[i].m_fWght = 0;
				}
				else
				{
					vec_attr_order[i].m_fWght /= 2;
					--m_vecGenLevel[cur_attr];
					m_lstEqvClass.swap( done_lst_eqvcls );
					temp_lst_eqvcls.clear();
					done_lst_eqvcls.clear();
					break;
				}
			}
			cout << "Current attribute: " << cur_attr << endl;
		}

		m_vecPtEqv.clear();
		m_vecPtEqv.insert( m_vecPtEqv.begin(), m_pMicrodata->GetTupleNum(), NULL );
		list<EqvClass>::iterator eqv_pos;
		list<long>::const_iterator tpl_pos;
		for (eqv_pos = m_lstEqvClass.begin(); eqv_pos != m_lstEqvClass.end(); ++eqv_pos)
		{
			EqvClass& r_eqv = *eqv_pos;
			for (tpl_pos = r_eqv.m_lstTplId.begin(); tpl_pos != r_eqv.m_lstTplId.end(); ++tpl_pos)
			{
				m_vecPtEqv[*tpl_pos] = &r_eqv;
			}
		}

		m_bGenExists = true;
	}
	catch( AnonyException e )
	{
		m_lstEqvClass.clear();
		m_vecGenLevel.clear();
		m_vecPtEqv.clear();
		cout << e.GetMsg() << endl;
		m_bGenExists = false;
		throw( e );
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////


bool Generalization::EvaluateRisk( const vector<long>& rVecAttr, Verifier& rVerifier )
{
	try
	{
		AnonyException temp_exception;
		temp_exception.m_bShallContinue = true;
		stringstream temp_sstream;

		if (rVecAttr.size() > m_vecQiAttr.size())
		{
			temp_sstream << "Generalization::EvaluateRisk Error: Incorrect number of attributes";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );			
		}

		vector<long> vec_attr = rVecAttr;
		sort( vec_attr.begin(), vec_attr.end() );

		long i, j;

		j = 0;
		for (i = 0; i < vec_attr.size(); ++i)
		{
			while (m_vecQiAttr[j].m_nAttr < vec_attr[i] && m_vecQiAttr[j].m_nAttr != vec_attr[i])
			{
				++j;
				if (j == m_vecQiAttr.size())
				{
					temp_sstream << "Generalization::EvaluateRisk Error: Input attributes are not all quasi-identifiers";
					temp_exception.SetMsg( temp_sstream.str() );
					throw( temp_exception );			
				}				
			}
		}

		const vector<Metadata>& r_meta = m_pMicrodata->GetVecMeta();
		const vector<UnitValue>& r_micro = m_pMicrodata->GetVecTuple();
		const long attr_num = r_meta.size();
		const long tuple_num = r_micro.size() / attr_num;
		const long qi_num = m_vecQiAttr.size();

		m_vecRisk.clear();
		m_vecRisk.insert( m_vecRisk.end(), tuple_num, 0 );

		list<EqvClass>::iterator eqv_pos;
		list<long>::const_iterator tpl_pos;

		if (rVecAttr.size() == m_vecQiAttr.size())
		{
			vector<EqvClass*> vec_pt_eqv;
			vec_pt_eqv.push_back( NULL );
			for (eqv_pos = m_lstEqvClass.begin(); eqv_pos != m_lstEqvClass.end(); ++eqv_pos)
			{
				vec_pt_eqv[0] = &(*eqv_pos);
				rVerifier.SetRisk( vec_pt_eqv, m_vecRisk );
			}
		}
		else
		{
			pair< vector<UnitValue>, vector<EqvClass*> > key_pair;
			key_pair.first.insert( key_pair.first.end(), rVecAttr.size(), UnitValue() );
			map<vector< UnitValue>, vector<EqvClass*> > emap;
			map<vector< UnitValue>, vector<EqvClass*> >::iterator emap_pos;
			vector<map< vector<UnitValue>, vector<EqvClass*> >::iterator> vec_pos;
			vec_pos.reserve( m_lstEqvClass.size() );
			pair< map<vector<UnitValue>, vector<EqvClass*> >::iterator, bool> rslt_pair;
			map<UnitValue, long>::iterator sv_pos;

			for (eqv_pos = m_lstEqvClass.begin(); eqv_pos != m_lstEqvClass.end(); ++eqv_pos)
			{
				EqvClass& r_eqv = *eqv_pos;
				for (j = 0; j < rVecAttr.size(); ++j)
				{					
					if (r_eqv.m_vecNodePos[rVecAttr[j]] != -1)
					{
						key_pair.first[j] = r_eqv.m_vecNodePos[rVecAttr[j]];
					}
					else
					{
						key_pair.first[j] = r_micro[r_eqv.m_lstTplId.front() * attr_num + rVecAttr[j]];
					}
				}

				emap_pos = emap.find( key_pair.first );
				if (emap_pos == emap.end())
				{
					rslt_pair = emap.insert( key_pair );
					(*rslt_pair.first).second.push_back( &r_eqv );
					vec_pos.push_back( rslt_pair.first );
				}
				else
				{
					(*emap_pos).second.push_back( &r_eqv );
				}
			}

			for (emap_pos = emap.begin(); emap_pos != emap.end(); ++emap_pos)
			{
				rVerifier.SetRisk( (*emap_pos).second, m_vecRisk );
			}			
		}

		for (i = 0; i < tuple_num; ++i)
		{
			if (m_vecStatus[i] != anony::NORMAL)
			{
				m_vecRisk[i] = 0;
			}
		}
	}
	catch( AnonyException e )
	{
		m_vecRisk.clear();
		cout << e.GetMsg() << endl;
		throw( e );
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////


void Generalization::EvaluateRisk( const vector<long>& rVecAttr, Verifier& rVerifier, long nBinNum, vector<long>& rVecCnt, float& fMaxRisk, float fMinRisk, bool bNeedsUpdate )
{
	if (bNeedsUpdate == true)
	{
		EvaluateRisk( rVecAttr, rVerifier );
	}

	double maxRisk = 0;

	long i, j;
	long total_num = m_vecRisk.size();

	for (i = 0; i < total_num; ++i)
	{
		if ( m_vecRisk[i] > maxRisk )
		{
			maxRisk = m_vecRisk[i];
		}
	}

	if ( maxRisk > 0.9)
	{
		fMaxRisk = 1.0;
	}
	else if ( maxRisk > 0.8)
	{
		fMaxRisk = 0.9;
	}
	else if ( maxRisk > 0.7)
	{
		fMaxRisk = 0.8;
	}
	else if ( maxRisk > 0.6)
	{
		fMaxRisk = 0.7;
	}
	else if ( maxRisk > 0.5)
	{
		fMaxRisk = 0.6;
	}
	else if ( maxRisk > 0.4)
	{
		fMaxRisk = 0.5;
	}
	else if ( maxRisk > 0.3)
	{
		fMaxRisk = 0.4;
	}
	else if ( maxRisk > 0.2)
	{
		fMaxRisk = 0.3;
	}
	else if ( maxRisk > 0.1)
	{
		fMaxRisk = 0.2;
	}
	else
	{
		fMaxRisk = 0.1;
	}

	rVecCnt.clear();
	rVecCnt.insert( rVecCnt.begin(), nBinNum, 0 );

	for (i = 0; i < total_num; ++i)
	{
		if (  m_vecRisk[i] - fMinRisk >= 0 )
		{
			j = min( nBinNum - 1, long( ( m_vecRisk[i] - fMinRisk ) * nBinNum / ( fMaxRisk - fMinRisk ) ) );
			++rVecCnt[j];
		}
	}

	return;
}



//////////////////////////////////////////////////////////////////////////


void Generalization::Delete( float fThreshold )
{
	long i;
	long total_num = m_vecRisk.size();
	for (i = 0; i < total_num; ++i)
	{
		if (m_vecRisk[i] >= fThreshold)
		{
			m_vecStatus[i] = anony::DELETED;
			m_setDeleted.insert( i );
			m_vecRisk[i] = 0;
		}
	}

	m_bRootDirty = true;
}


//////////////////////////////////////////////////////////////////////////


void Generalization::DelTopRisk( const long nTopK )
{
	long i;
	long total_num = m_vecRisk.size();

	RiskIdPair temp_pair;
	vector<RiskIdPair> vec_pair;

	vec_pair.insert( vec_pair.end(), total_num, temp_pair );	

	for (i = 0; i < total_num; ++i)
	{
		vec_pair[i].m_nTplId = i;
		vec_pair[i].m_fRisk = m_vecRisk[i];
	}

	nth_element( vec_pair.begin(), vec_pair.begin() + nTopK, vec_pair.end(), greater<RiskIdPair>() );

	for (i = 0; i < nTopK; ++i)
	{
		long j = vec_pair[i].m_nTplId;
		m_vecStatus[j] = anony::DELETED;
		m_setDeleted.insert( j );
		m_vecRisk[j] = 0;
	}

	m_bRootDirty = true;
}


//////////////////////////////////////////////////////////////////////////


void Generalization::Clean( vector<long>& rVecId)
{	
	if ( !m_vecStatus.empty() )
	{
		long i;
		long size = rVecId.size();
		vector<long> vec_temp = rVecId;

		rVecId.clear();
		rVecId.reserve( size );

		for (i = 0; i < vec_temp.size(); ++i)
		{
			if ( m_vecStatus[vec_temp[i]] !=  anony::DELETED )
			{
				rVecId.push_back( vec_temp[i] );
			}
		}
	}

	return;
}


//////////////////////////////////////////////////////////////////////////


void Generalization::Resume( vector<long>& rVecId)
{
	long i;
	long size = rVecId.size();
	vector<long> vec_temp = rVecId;

	rVecId.clear();
	rVecId.reserve( size );

	for (i = 0; i < m_pMicrodata->GetTupleNum(); ++i)
	{
		rVecId.push_back( i );
	}

	return;
}


//////////////////////////////////////////////////////////////////////////


void Generalization::RecoverAll()
{
	set<long>::iterator pos_set;
	for (pos_set = m_setDeleted.begin(); pos_set != m_setDeleted.end(); ++pos_set)
	{
		m_vecStatus[*pos_set] = anony::NORMAL;
	}
	m_setDeleted.clear();

	m_bRootDirty = true;
}


//////////////////////////////////////////////////////////////////////////


const QString& Generalization::GetValueQstr( long nTupleIdx, long nAttrIdx )
{
	if (m_pMicrodata == NULL)
	{
		AnonyException temp_exception;
		temp_exception.m_bShallContinue = true;
		temp_exception.SetMsg( "Generalization::GetValueStr Error: No microdata is specified" );
		throw( temp_exception );
	}

	if ((nTupleIdx < 0) || (nTupleIdx >= m_pMicrodata->GetTupleNum()) || (nAttrIdx < 0) || (nAttrIdx >= m_pMicrodata->GetAttrNum()))
	{
		AnonyException temp_exception;
		temp_exception.m_bShallContinue = true;
		temp_exception.SetMsg( "Generalization::GetValueStr Error: Incorrect value index" );
		throw( temp_exception );
	}

	if (m_vecPtEqv.size() != m_pMicrodata->GetTupleNum())
	{
		AnonyException temp_exception;
		temp_exception.m_bShallContinue = true;
		temp_exception.SetMsg( "Generalization::GetValueStr Error: No generalization has been performed" );
		throw( temp_exception );
	}

	m_qstrTemp.clear();
	
	if (m_vecStatus[nTupleIdx] != anony::NORMAL)
	{
		return m_pMicrodata->GetValueQstr( nTupleIdx, nAttrIdx );
	}

	EqvClass* p_eqv = m_vecPtEqv[nTupleIdx];
	if ((m_vecQiInd[nAttrIdx] < 0) || (p_eqv->m_vecNodePos[nAttrIdx] < 0))
	{
		return m_pMicrodata->GetValueQstr( nTupleIdx, nAttrIdx );
	}
	else
	{	
		const HrchNode& r_node = m_vecHrch[m_vecQiInd[nAttrIdx]].GetNode( p_eqv->m_vecNodePos[nAttrIdx] );
		m_qstrTemp = r_node.m_strName.c_str();
		return m_qstrTemp;
	}
}


//////////////////////////////////////////////////////////////////////////


bool Generalization::GetJointDensity( const long nAttrX, const long nAttrY, long& nWidth, long& nHeight, vector<float>& rVecOri, vector<float>& rVecAnony, vector<QString>& rVecNameX, vector<QString>& rVecNameY, float& fheightMax )
{
	const vector<Metadata>& r_meta = m_pMicrodata->GetVecMeta();
	const vector<UnitValue>& r_micro = m_pMicrodata->GetVecTuple();
	const long attr_num = r_meta.size();
	const long tuple_num = r_micro.size() / attr_num;
	const long qi_num = m_vecQiAttr.size();

	if ((r_meta[nAttrX].m_nType == anony::FLOAT) || (r_meta[nAttrY].m_nType == anony::FLOAT))
	{
		return false;
	}

	long i, j, k, l;

	rVecOri.clear();
	rVecAnony.clear();

	long x_min = r_meta[nAttrX].m_nMinValue.l;
	long x_max = r_meta[nAttrX].m_nMaxValue.l;
	long y_min = r_meta[nAttrY].m_nMinValue.l;
	long y_max = r_meta[nAttrY].m_nMaxValue.l;
	long x_size, y_size;

	x_size = x_max - x_min + 1;
	y_size = y_max - y_min + 1;

	vector<long> vec_code_x, vec_code_y;
	vec_code_x.insert( vec_code_x.end(), x_size, -1 );
	vec_code_y.insert( vec_code_y.end(), y_size, -1 );

	tr1::unordered_map<long, string>::const_iterator pos_map;
	QString temp_str;

	rVecNameX.clear();
	if (r_meta[nAttrX].m_nType == anony::INTEGER)
	{
		for (i = 0; i < x_size; ++i)
		{
			vec_code_x[i] = i;
		}
		nWidth = x_size;

		rVecNameX.reserve( nWidth );
		for (i = 0; i < nWidth; ++i)
		{
			temp_str.setNum( i );
			rVecNameX.push_back( temp_str );
		}
	}
	else
	{
		const tr1::unordered_map<long, string>& r_x_map = r_meta[nAttrX].m_mapValueName;
		nWidth = r_meta[nAttrX].m_mapValueName.size();

		rVecNameX.reserve( nWidth );
		for (pos_map = r_x_map.begin(); pos_map != r_x_map.end(); ++pos_map)
		{
			vec_code_x[(*pos_map).first - x_min] = 0;
		}

		j = 0;
		for (i = 0; i < x_size; ++i)
		{
			if (vec_code_x[i] >= 0)
			{
				vec_code_x[i] = j;
				++j;

				temp_str = r_x_map.find( i + x_min )->second.c_str();
				rVecNameX.push_back( temp_str );
			}
		}
	}

	rVecNameY.clear();
	if (r_meta[nAttrY].m_nType == anony::INTEGER)
	{
		for (i = 0; i < y_size; ++i)
		{
			vec_code_y[i] = i;
		}
		nHeight = y_size;

		rVecNameY.reserve( nHeight );
		for (i = 0; i < nHeight; ++i)
		{
			temp_str.setNum( i );
			rVecNameY.push_back( temp_str );
		}
	}
	else
	{
		const tr1::unordered_map<long, string>& r_y_map = r_meta[nAttrY].m_mapValueName;
		nHeight = r_meta[nAttrY].m_mapValueName.size();

		rVecNameY.reserve( nHeight );
		for (pos_map = r_y_map.begin(); pos_map != r_y_map.end(); ++pos_map)
		{
			vec_code_y[(*pos_map).first - y_min] = 0;
		}

		j = 0;
		for (i = 0; i < y_size; ++i)
		{
			if (vec_code_y[i] >= 0)
			{
				vec_code_y[i] = j;
				++j;

				temp_str = (r_y_map.find( i + y_min ))->second.c_str();
				rVecNameY.push_back( temp_str );
			}
		}
	}

	rVecOri.insert( rVecOri.begin(), nHeight * nWidth, 0 );
	rVecAnony.insert( rVecAnony.begin(), nHeight * nWidth, 0 );

	long x_idx;
	long y_idx;
	float height_max = 0;
	for (i = 0; i < tuple_num; ++i)
	{
		if (m_vecStatus[i] == anony::CANTGEN)
		{
			continue;
		}
		x_idx = vec_code_x[r_micro[i * attr_num + nAttrX].l - x_min];
		y_idx = vec_code_y[r_micro[i * attr_num + nAttrY].l - y_min];
		rVecOri[y_idx * nWidth + x_idx] = rVecOri[y_idx * nWidth + x_idx] + 1;
		if (rVecOri[y_idx * nWidth + x_idx] > height_max)
		{
			height_max = rVecOri[y_idx * nWidth + x_idx];
		}
	}

	for (i = 0; i < nWidth * nHeight; ++i)
	{
		rVecOri[i] /= height_max;
	}

	long x_QI, y_QI;
	x_QI = y_QI = -1;

// 	for (i = 0; i < qi_num; ++i)
// 	{
// 		if (nAttrX == m_vecQiAttr[i].m_nAttr)
// 		{
// 			x_QI = i;
// 		}
// 
// 		if (nAttrY == m_vecQiAttr[i].m_nAttr)
// 		{
// 			y_QI = i;
// 		}
// 	}
// 
// 	if (x_QI >= 0)
// 	{
// 		if (m_vecGenLevel[x_QI] == 0)
// 		{
// 			x_QI = -1;
// 		}
// 	}
// 
// 	if (y_QI >= 0)
// 	{
// 		if (m_vecGenLevel[y_QI] == 0)
// 		{
// 			y_QI = -1;
// 		}
// 	}

	x_QI = nAttrX;
	y_QI = nAttrY;

	if (m_vecGenLevel[nAttrX] <= 0)
	{
		x_QI = -1;
	}

	if (m_vecGenLevel[nAttrY] <= 0)
	{
		y_QI = -1;
	}

	if ((x_QI < 0) && (y_QI < 0))
	{
		rVecAnony = rVecOri;
		fheightMax = height_max;
		return true;
	}

	long gen_x, gen_y;
	long base_x, base_y;
	if (x_QI >= 0)
	{
		gen_x = m_vecHrch[nAttrX].GetCount( m_vecGenLevel[x_QI] - 1 );
		base_x = (m_vecHrch[nAttrX].GetVecLevel())[m_vecGenLevel[x_QI] - 1];
	}
	else
	{
		gen_x = nWidth;
		base_x = x_min;
	}

	if (y_QI >= 0)
	{
		gen_y = m_vecHrch[nAttrY].GetCount( m_vecGenLevel[y_QI] - 1 );
		base_y = (m_vecHrch[nAttrY].GetVecLevel())[m_vecGenLevel[y_QI] - 1];
	}
	else
	{
		gen_y = nHeight;
		base_y = y_min;
	}

	vector<long> vec_temp;
	vec_temp.insert( vec_temp.begin(), gen_x * gen_y, 0 );

	list<EqvClass>::iterator eqv_pos;
	list<long>::const_iterator tpl_pos;
	long tpl_no;

	for (eqv_pos = m_lstEqvClass.begin(); eqv_pos != m_lstEqvClass.end(); ++eqv_pos)
	{
		const EqvClass& r_eqv = *eqv_pos;
		long x, y;

		for (tpl_pos = r_eqv.m_lstTplId.begin(); tpl_pos != r_eqv.m_lstTplId.end(); ++tpl_pos)
		{
			tpl_no = *tpl_pos;

			if (m_vecStatus[tpl_no] != anony::NORMAL)
			{
				continue;
			}

			if (x_QI >= 0)
			{
				x = r_eqv.m_vecNodePos[nAttrX] - base_x;			
			}
			else
			{
				x = vec_code_x[r_micro[tpl_no * attr_num + nAttrX].l - x_min];
			}

			if (y_QI >= 0)
			{
				y = r_eqv.m_vecNodePos[nAttrY] - base_y;				
			}
			else
			{
				y = vec_code_y[r_micro[tpl_no * attr_num + nAttrY].l - y_min];
			}
			++vec_temp[y * gen_x + x];
		}
	}


	vector<long> vec_gen_x, vec_gen_y;
	vec_gen_x.insert( vec_gen_x.end(), 2 * gen_x, -1 );
	vec_gen_y.insert( vec_gen_y.end(), 2 * gen_y, -1 );

	if (x_QI < 0)
	{
		for (i = 0; i < gen_x; ++i)
		{
			vec_gen_x[2 * i] = vec_gen_x[2 * i + 1] = i;
		}
	}
	else
	{
		for (i = 0; i < gen_x; ++i)
		{
			for (j = (m_vecHrch[nAttrX].GetVecNode())[base_x + i].m_uInterval.m_uLeft.l - x_min; j < x_size; ++j)
			{
				if (vec_code_x[j] != -1)
				{
					vec_gen_x[2 * i] = vec_code_x[j];
					break;
				}
			}
			for (j = (m_vecHrch[nAttrX].GetVecNode())[base_x + i].m_uInterval.m_uRight.l - x_min; j >=0; --j)
			{
				if (vec_code_x[j] != -1)
				{
					vec_gen_x[2 * i + 1] = vec_code_x[j];
					break;
				}
			}			
		}
	}

	if (y_QI < 0)
	{
		for (i = 0; i < gen_y; ++i)
		{
			vec_gen_y[2 * i] = vec_gen_y[2 * i + 1] = i;
		}
	}
	else
	{
		for (i = 0; i < gen_y; ++i)
		{
			for (j = (m_vecHrch[nAttrY].GetVecNode())[base_y + i].m_uInterval.m_uLeft.l - y_min; j < y_size; ++j)
			{
				if (vec_code_y[j] != -1)
				{
					vec_gen_y[2 * i] = vec_code_y[j];
					break;
				}
			}
			for (j = (m_vecHrch[nAttrY].GetVecNode())[base_y + i].m_uInterval.m_uRight.l - y_min; j >=0; --j)
			{
				if (vec_code_y[j] != -1)
				{
					vec_gen_y[2 * i + 1] = vec_code_y[j];
					break;
				}
			}			
		}
	}


	float mass;
	for (j = 0; j < gen_y; ++j)
	{
		for (i = 0; i < gen_x; ++i)
		{
			mass = float( vec_temp[j * gen_x + i] ) / (vec_gen_x[2 * i + 1] - vec_gen_x[2 * i] + 1) / (vec_gen_y[2 * j + 1] - vec_gen_y[2 * j] + 1) / height_max;

			for (k = vec_gen_y[2 * j]; k <= vec_gen_y[2 * j + 1]; ++k)
			{
				for (l = vec_gen_x[2 * i]; l <= vec_gen_x[2 * i + 1]; ++l)
				{
					rVecAnony[k * nWidth + l] += mass;
				}
			}
		}
	}

	fheightMax = height_max;

	return true;
}


//////////////////////////////////////////////////////////////////////////


bool Generalization::GetMarginalDensity( const long nAttr, long& nLength, vector<float>& rVecOri, vector<float>& rVecAnony, vector<QString>& rVecName )
{
	const vector<Metadata>& r_meta = m_pMicrodata->GetVecMeta();
	const vector<UnitValue>& r_micro = m_pMicrodata->GetVecTuple();
	const long attr_num = r_meta.size();
	const long tuple_num = r_micro.size() / attr_num;
	const long qi_num = m_vecQiAttr.size();

	if ( r_meta[nAttr].m_nType == anony::FLOAT )
	{
		return false;
	}

	long i, j, k, l;

	rVecOri.clear();
	rVecAnony.clear();

	long min = r_meta[nAttr].m_nMinValue.l;
	long max = r_meta[nAttr].m_nMaxValue.l;
	long size;

	size = max - min + 1;
	
	vector<long> vec_code;
	vec_code.insert( vec_code.end(), size, -1 );

	tr1::unordered_map<long, string>::const_iterator pos_map;
	QString temp_str;

	rVecName.clear();
	if (r_meta[nAttr].m_nType == anony::INTEGER)
	{
		for (i = 0; i < size; ++i)
		{
			vec_code[i] = i;
		}
		nLength = size;

		rVecName.reserve( nLength );
		for (i = 0; i < nLength; ++i)
		{
			temp_str.setNum( i );
			rVecName.push_back( temp_str );
		}
	}
	else
	{
		const tr1::unordered_map<long, string>& r_map = r_meta[nAttr].m_mapValueName;
		nLength = r_meta[nAttr].m_mapValueName.size();

		rVecName.reserve( nLength );
		for (pos_map = r_map.begin(); pos_map != r_map.end(); ++pos_map)
		{
			vec_code[(*pos_map).first - min] = 0;
		}

		j = 0;
		for (i = 0; i < size; ++i)
		{
			if (vec_code[i] >= 0)
			{
				vec_code[i] = j;
				++j;

				temp_str = r_map.find( i + min )->second.c_str();
				rVecName.push_back( temp_str );
			}
		}
	}

	rVecOri.insert( rVecOri.begin(), nLength, 0 );
	rVecAnony.insert( rVecAnony.begin(), nLength, 0 );

	long idx;
	float height_max = 0;
	for (i = 0; i < tuple_num; ++i)
	{
		if (m_vecStatus[i] == anony::CANTGEN)
		{
			continue;
		}
		idx = vec_code[r_micro[i * attr_num + nAttr].l - min];
		rVecOri[idx] = rVecOri[idx] + 1;
		if (rVecOri[idx] > height_max)
		{
			height_max = rVecOri[idx];
		}
	}

	for (i = 0; i < nLength; ++i)
	{
		rVecOri[i] /= height_max;
	}

	long QI = -1;

// 	for (i = 0; i < qi_num; ++i)
// 	{
// 		if (nAttr == m_vecQiAttr[i].m_nAttr)
// 		{
// 			QI = i;
// 		}
// 	}
// 
// 	if (QI >= 0)
// 	{
// 		if (m_vecGenLevel[QI] == 0)
// 		{
// 			QI = -1;
// 		}
// 	}

	QI = nAttr;
	
	if (m_vecGenLevel[nAttr] <= 0)
	{
		QI = -1;
	}

	if (QI < 0)
	{
		rVecAnony = rVecOri;
		return true;
	}

	long gen;
	long base;
	if (QI >= 0)
	{
		gen = m_vecHrch[nAttr].GetCount( m_vecGenLevel[QI] - 1 );
		base = (m_vecHrch[nAttr].GetVecLevel())[m_vecGenLevel[QI] - 1];
	}
	else
	{
		gen = nLength;
		base = min;
	}

	vector<long> vec_temp;
	vec_temp.insert( vec_temp.begin(), gen, 0 );

	list<EqvClass>::iterator eqv_pos;
	list<long>::const_iterator tpl_pos;
	long tpl_no;

	for (eqv_pos = m_lstEqvClass.begin(); eqv_pos != m_lstEqvClass.end(); ++eqv_pos)
	{
		const EqvClass& r_eqv = *eqv_pos;
		long x;

		for (tpl_pos = r_eqv.m_lstTplId.begin(); tpl_pos != r_eqv.m_lstTplId.end(); ++tpl_pos)
		{
			tpl_no = *tpl_pos;

			if (m_vecStatus[tpl_no] != anony::NORMAL)
			{
				continue;
			}

			if (QI >= 0)
			{
				x = r_eqv.m_vecNodePos[nAttr] - base;
			}
			else
			{
				x = vec_code[r_micro[tpl_no * attr_num + nAttr].l - min];
			}

			++vec_temp[x];
		}
	}

	vector<long> vec_gen;
	vec_gen.insert( vec_gen.end(), 2 * gen, -1 );

	if (QI < 0)
	{
		for (i = 0; i < gen; ++i)
		{
			vec_gen[2 * i] = vec_gen[2 * i + 1] = i;
		}
	}
	else
	{
		for (i = 0; i < gen; ++i)
		{
			for (j = (m_vecHrch[nAttr].GetVecNode())[base + i].m_uInterval.m_uLeft.l - min; j < size; ++j)
			{
				if (vec_code[j] != -1)
				{
					vec_gen[2 * i] = vec_code[j];
					break;
				}
			}
			for (j = (m_vecHrch[nAttr].GetVecNode())[base + i].m_uInterval.m_uRight.l - min; j >=0; --j)
			{
				if (vec_code[j] != -1)
				{
					vec_gen[2 * i + 1] = vec_code[j];
					break;
				}
			}			
		}
	}

	float mass;
	for (j = 0; j < gen; ++j)
	{
		mass = float( vec_temp[j] ) / (vec_gen[2 * j + 1] - vec_gen[2 * j] + 1) / height_max;

		for (k = vec_gen[2 * j]; k <= vec_gen[2 * j + 1]; ++k)
		{
			rVecAnony[k] += mass;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////


void Generalization::SortByRisk( vector<long>& rVecId, bool bAscend )
{	
	const long tuple_num =  m_pMicrodata->GetTupleNum();

	long i;
	RiskIdPair temp_pair;
	vector<RiskIdPair> vec_pair;
	
	vec_pair.insert( vec_pair.end(), tuple_num, temp_pair );	

	for (i = 0; i < tuple_num; ++i)
	{
		vec_pair[i].m_nTplId = i;
		vec_pair[i].m_fRisk = m_vecRisk[i];
	}

	if (bAscend == true)
	{
		sort( vec_pair.begin(), vec_pair.end() );
	}
	else
	{
		sort( vec_pair.begin(), vec_pair.end(), greater<RiskIdPair>() );
	}

	rVecId.clear();
	rVecId.reserve( tuple_num );
	for (i = 0; i < tuple_num; ++i)
	{
		rVecId.push_back( vec_pair[i].m_nTplId );
	}

	return;
}


/************************************************************************/
/* Verifier Classes: L-Diversity and T-Closeness                                                                     */
/************************************************************************/

//////////////////////////////////////////////////////////////////////////


bool L_Diversity::Verify( const EqvClass& rEqvCls )
{
	if (m_nL > rEqvCls.m_mapSenCnt.size())
	{
		return false;
	}

	m_vecSenCnt.clear();
	m_vecSenCnt.reserve( rEqvCls.m_mapSenCnt.size() );

	map<UnitValue, long>::const_iterator sen_pos;
	for (sen_pos =  rEqvCls.m_mapSenCnt.begin(); sen_pos != rEqvCls.m_mapSenCnt.end(); ++sen_pos)
	{
		m_vecSenCnt.push_back( (*sen_pos).second );
	}
	sort(  m_vecSenCnt.begin(),  m_vecSenCnt.end(), greater<long>() );

	long i;
	long temp_cnt = 0;
	for (i = 0; i < m_nL - 1; ++i)
	{
		temp_cnt +=  m_vecSenCnt[i];
	}

	if (m_vecSenCnt[0] >= m_fC * (rEqvCls.m_lstTplId.size() - temp_cnt))
	{
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////


bool L_Diversity::CheckBeforeAnonymize( const Generalization& rGen )
{
	AnonyException temp_exception;
	temp_exception.m_bShallContinue = true;
	stringstream temp_sstream;

	if (rGen.m_pMicrodata == NULL)
	{
		temp_sstream << "Generalization::CheckBeforeVerify Error: Pointer to the microdata is NULL";
		temp_exception.SetMsg( temp_sstream.str() );
		throw( temp_exception );		
	}

	if (rGen.m_eqvRootClass.m_lstTplId.size() == 0)
	{
		temp_sstream << "Generalization::CheckBeforeVerify Error: Root equivalence class has not been initialized";
		temp_exception.SetMsg( temp_sstream.str() );
		throw( temp_exception );			
	}

	if (m_nL < 2 || m_fC <= 0)
	{
		temp_sstream << "Generalization::CheckBeforeVerify Error: Incorrect value for l or c";
		temp_exception.SetMsg( temp_sstream.str() );
		throw( temp_exception );
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////


bool L_Diversity::SetRisk( const vector<EqvClass*>& rVecPtEqv, vector<float>& rVecRisk )
{
	long num_eqv = rVecPtEqv.size();
	long i;

	m_mapSenCnt = (rVecPtEqv[0])->m_mapSenCnt;

	map<UnitValue, long>::const_iterator sv_pos;
	for (i = 1; i < num_eqv; ++i)
	{
		const map<UnitValue, long>& r_sa_map = (rVecPtEqv[i])->m_mapSenCnt;
		for (sv_pos = r_sa_map.begin(); sv_pos != r_sa_map.end(); ++sv_pos)
		{
			map<UnitValue, long>::iterator temp_pos = m_mapSenCnt.find( (*sv_pos).first );
			if (temp_pos == m_mapSenCnt.end())
			{
				m_mapSenCnt.insert( *sv_pos );
			}
			else
			{
				(*temp_pos).second += (*sv_pos).second;
			}
		}
	}

	m_vecSenCnt.clear();
	m_vecSenCnt.reserve( m_mapSenCnt.size() );

	map<UnitValue, long>::const_iterator sen_pos;
	long total_num = 0;
	for (sen_pos =  m_mapSenCnt.begin(); sen_pos != m_mapSenCnt.end(); ++sen_pos)
	{
		m_vecSenCnt.push_back( (*sen_pos).second );
		total_num += m_vecSenCnt.back();
	}
	sort(  m_vecSenCnt.begin(),  m_vecSenCnt.end(), greater<long>() );

	long value_num = min( long( m_vecSenCnt.size() ), m_nBckKnw + 1 );
	long temp_cnt = 0;
	for (i = 1; i < value_num; ++i)
	{
		temp_cnt += m_vecSenCnt[i];
	}

	double temp_risk = m_vecSenCnt[0] / double(total_num - temp_cnt);	

	list<long>::const_iterator tpl_pos;
	for (i = 0; i < num_eqv; ++i)
	{
		const EqvClass& r_eqv = *rVecPtEqv[i];
		for (tpl_pos = r_eqv.m_lstTplId.begin(); tpl_pos != r_eqv.m_lstTplId.end(); ++tpl_pos)
		{
			rVecRisk[*tpl_pos] = temp_risk;
		}
	}	

	return true;
}


//////////////////////////////////////////////////////////////////////////


bool L_Diversity::CheckBeforeEvalRisk( const Generalization& rGen )
{
	AnonyException temp_exception;
	temp_exception.m_bShallContinue = true;
	stringstream temp_sstream;

	if (m_nBckKnw < 0)
	{
		temp_sstream << "Generalization::CheckBeforeEvalRisk Error: Incorrect pieces of background knowledge";
		temp_exception.SetMsg( temp_sstream.str() );
		throw( temp_exception );			
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////

T_Closeness::T_Closeness( const float t, const Generalization& rGen )
{
	m_fT = t;

	// set the sensitive attribute's meta information
	m_nSenType = rGen.m_pMicrodata->GetVecMeta()[rGen.m_nSenAttr].m_nType;
	m_nTuples = rGen.m_pMicrodata->GetTupleNum();
	m_mapSenDistn.clear();
	m_vecSenId.clear();

	// compute the probability distribution of the sensitive attributes
	switch( m_nSenType )
	{
	case anony::INTEGER :
		{
			for ( int t_index = 0; t_index < rGen.m_pMicrodata->GetTupleNum(); t_index++ )
			{
				UnitValue value = rGen.m_pMicrodata->GetValue( t_index, rGen.m_nSenAttr );
				
				map<UnitValue, long>::iterator iter = m_mapSenDistn.find( value );
				if ( iter == m_mapSenDistn.end() )
				{
					pair<UnitValue, long> temp_map;
					temp_map.first = value;
					temp_map.second = 1;
					m_mapSenDistn.insert( temp_map );
					m_vecSenId.push_back( value );
				}
				else
				{
					(*iter).second = (*iter).second ++;
				}
			}
			sort( m_vecSenId.begin(), m_vecSenId.end() );
			
			break;
		}

	case anony::FLOAT :
		{
			for ( int t_index = 0; t_index < rGen.m_pMicrodata->GetTupleNum(); t_index++ )
			{
				UnitValue value = rGen.m_pMicrodata->GetValue( t_index, rGen.m_nSenAttr );

				map<UnitValue, long>::iterator iter = m_mapSenDistn.find( value );
				if ( iter == m_mapSenDistn.end() )
				{
					pair<UnitValue, long> temp_map;
					temp_map.first = value;
					temp_map.second = 1;
					m_mapSenDistn.insert( temp_map );
					m_vecSenId.push_back( value );
				}
				else
				{
					(*iter).second = (*iter).second++;
				}
			}
			sort( m_vecSenId.begin(), m_vecSenId.end() );

			break;
		}

	case anony::CATEGORICAL :
		{
			for ( int t_index = 0; t_index < rGen.m_pMicrodata->GetTupleNum(); t_index++ )
			{
				UnitValue value = rGen.m_pMicrodata->GetValue( t_index, rGen.m_nSenAttr );

				map<UnitValue, long>::iterator iter = m_mapSenDistn.find( value );
				if ( iter == m_mapSenDistn.end() )
				{
					pair<UnitValue, long> temp_map;
					temp_map.first = value;
					temp_map.second = 1;
					m_mapSenDistn.insert( temp_map );
					m_vecSenId.push_back( value );
				}
				else
				{
					(*iter).second = (*iter).second++;
				}
			}
			break;
		}
	}	
}


//////////////////////////////////////////////////////////////////////////

bool T_Closeness::Verify( const EqvClass& rEqvCls )
{
	long total = 0;

	map<UnitValue, long>::const_iterator sen_pos;
	for (sen_pos =  rEqvCls.m_mapSenCnt.begin(); sen_pos != rEqvCls.m_mapSenCnt.end(); ++sen_pos)
	{
		total += (*sen_pos).second;
	}

	float emd = EMD( m_nTuples, rEqvCls.m_lstTplId.size(), m_vecSenId, m_mapSenDistn, rEqvCls.m_mapSenCnt );

	if ( emd > m_fT )
	{
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////

bool T_Closeness::CheckBeforeAnonymize( const Generalization& rGen )
{
	AnonyException temp_exception;
	temp_exception.m_bShallContinue = true;
	stringstream temp_sstream;

	if (rGen.m_pMicrodata == NULL)
	{
		temp_sstream << "Generalization::CheckBeforeVerify Error: Pointer to the microdata is NULL";
		temp_exception.SetMsg( temp_sstream.str() );
		throw( temp_exception );		
	}

	if (rGen.m_eqvRootClass.m_lstTplId.size() == 0)
	{
		temp_sstream << "Generalization::CheckBeforeVerify Error: Root equivalence class has not been initialized";
		temp_exception.SetMsg( temp_sstream.str() );
		throw( temp_exception );			
	}

	if (m_fT <= 0)
	{
		temp_sstream << "Generalization::CheckBeforeVerify Error: Incorrect value for t";
		temp_exception.SetMsg( temp_sstream.str() );
		throw( temp_exception );
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////


bool T_Closeness::SetRisk( const vector<EqvClass*>& rVecPtEqv, vector<float>& rVecRisk )
{
	long num_eqv = rVecPtEqv.size();
	long num_tuple = 0;
	long i;

	map<UnitValue, long>::const_iterator sv_pos;

	map<UnitValue, long> mapEqClsCnt = (rVecPtEqv[0])->m_mapSenCnt;
	for (sv_pos = (rVecPtEqv[0])->m_mapSenCnt.begin(); sv_pos != (rVecPtEqv[0])->m_mapSenCnt.end(); ++sv_pos)
	{
// 		map<UnitValue, long>::iterator temp_pos = mapEqClsCnt.find( (*sv_pos).first );
// 		if (temp_pos == mapEqClsCnt.end())
// 		{
// 			mapEqClsCnt.insert( *sv_pos );
// 		}
// 		else
// 		{
// 			(*temp_pos).second += (*sv_pos).second;
// 		}
		num_tuple += (*sv_pos).second;
	}

	for (i = 1; i < num_eqv; ++i)
	{
		const map<UnitValue, long>& r_sa_map = (rVecPtEqv[i])->m_mapSenCnt;
		for (sv_pos = r_sa_map.begin(); sv_pos != r_sa_map.end(); ++sv_pos)
		{
			map<UnitValue, long>::iterator temp_pos = mapEqClsCnt.find( (*sv_pos).first );
			if (temp_pos == mapEqClsCnt.end())
			{
				mapEqClsCnt.insert( *sv_pos );
			}
			else
			{
				(*temp_pos).second += (*sv_pos).second;
			}
			num_tuple += (*sv_pos).second;
		}
	}

	float emd = EMD( m_nTuples, num_tuple, m_vecSenId, m_mapSenDistn, mapEqClsCnt );

	list<long>::const_iterator tpl_pos;
	for (i = 0; i < num_eqv; ++i)
	{
		const EqvClass& r_eqv = *rVecPtEqv[i];
		for (tpl_pos = r_eqv.m_lstTplId.begin(); tpl_pos != r_eqv.m_lstTplId.end(); ++tpl_pos)
		{
			rVecRisk[*tpl_pos] = emd;
		}
	}	

	return true;
}


//////////////////////////////////////////////////////////////////////////

bool T_Closeness::CheckBeforeEvalRisk( const Generalization& rGen )
{
	AnonyException temp_exception;
	temp_exception.m_bShallContinue = true;
	stringstream temp_sstream;

	return true;
}


//////////////////////////////////////////////////////////////////////////

float T_Closeness::EMD( long numTable, long numEqCls, vector<UnitValue>& vecId, map<UnitValue, long>& mapTable, const map<UnitValue, long>& mapEqCls )
{
	float emd = 0.0f;

	switch( m_nSenType )
	{
	case anony::INTEGER :
		{
			vector<UnitValue>::iterator id_pos;
			float r = 0.0f;
			for ( id_pos = vecId.begin(); (id_pos + 1) != vecId.end(); ++id_pos )
			{
				UnitValue value = *id_pos;
				map<UnitValue, long>::const_iterator eqcls_pos = mapEqCls.find( value );
				map<UnitValue, long>::iterator table_pos = mapTable.find( value );
				if ( eqcls_pos != mapEqCls.end() )
				{
					r += (float) (*table_pos).second / numTable - (float) (*eqcls_pos).second / numEqCls;
				}
				else
				{
					r += (float) (*table_pos).second / numTable;
				}
				float abs_r = abs(r);
				emd += abs_r;
			}
			emd = emd / ( vecId.size() - 1 );

			break;
		}

	case anony::FLOAT :
		{
			vector<UnitValue>::iterator id_pos;
			float r = 0.0f;
			for ( id_pos = vecId.begin(); (id_pos + 1) != vecId.end(); ++id_pos )
			{
				UnitValue value = *id_pos;
				map<UnitValue, long>::const_iterator eqcls_pos = mapEqCls.find( value );
				map<UnitValue, long>::iterator table_pos = mapTable.find( value );
				if ( eqcls_pos != mapEqCls.end() )
				{
					r += (float) (*table_pos).second / numTable - (float) (*eqcls_pos).second / numEqCls;
				}
				else
				{
					r += (float) (*table_pos).second / numTable;
				}
				float abs_r = abs(r);
				emd += abs_r;
			}
			emd = emd / ( vecId.size() - 1 );

			break;
		}

	case anony::CATEGORICAL :
		{
			vector<UnitValue>::iterator id_pos;
			float r = 0.0f;
			for ( id_pos = vecId.begin(); id_pos != vecId.end(); ++id_pos )
			{
				UnitValue value = *id_pos;
				map<UnitValue, long>::const_iterator eqcls_pos = mapEqCls.find( value );
				map<UnitValue, long>::iterator table_pos = mapTable.find( value );
				if ( eqcls_pos != mapEqCls.end() )
				{
					r = (float) (*table_pos).second / numTable - (float) (*eqcls_pos).second / numEqCls;
				}
				else
				{
					r = (float) (*table_pos).second / numTable;
				}

				if ( r > 0 )
				{
					emd += r;
				}
			}

			break;
		}
	}

	return emd;
}


//////////////////////////////////////////////////////////////////////////
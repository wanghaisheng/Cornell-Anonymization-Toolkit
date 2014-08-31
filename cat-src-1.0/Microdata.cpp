#include "stdafx.h"
#include "Microdata.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
#include <algorithm>
#include "AnonyException.h"


//////////////////////////////////////////////////////////////////////////


bool Microdata::ReadFiles( const char* pMicrodata, const char* pMetadata )
{
	ifstream micro_file( pMicrodata );
	ifstream meta_file( pMetadata );

	m_vecTuple.clear();
	m_vecMeta.clear();

	try
	{
		AnonyException temp_exception;
		temp_exception.m_bShallContinue = true;
		stringstream temp_sstream;
		if (micro_file.is_open() == false)
		{
			temp_sstream << "Microdata::ReadFiles Error: Cannot Open Microdata File: " << pMicrodata;
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}
		if (meta_file.is_open() == false)
		{
			temp_sstream << "Microdata::ReadFiles Error: Cannot Open Metadata File: " << pMetadata;
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}

		long attr_num;
		long tuple_num;

		//	Read the number of attributes
		meta_file >> attr_num;
		if (attr_num < 1 || attr_num > 1000 || meta_file.fail() == true)
		{
			temp_sstream << "Microdata::ReadFiles Error: The number of attributes is incorrect";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}

		//	Read the number of tuples
		meta_file >> tuple_num;
		if (micro_file.fail() == true || tuple_num < 1)
		{
			temp_sstream << "Microdata::ReadFiles Error: The number of tuple is smaller than 1";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}

		m_nAttrNum = attr_num;
		m_nTupleNum = tuple_num;

		meta_file.ignore( 10000, '\n' );

		//	Initialize the vector of metadata
		Metadata temp_meta;
		m_vecMeta.insert( m_vecMeta.end(), attr_num, temp_meta );

		long i, j, k;
		long domain_size, miss_num, attr_type;
		UnitValue temp_unit;

		//	Read the metadata for each attribute
		for (i = 0; i < attr_num; ++i)
		{
			meta_file >> domain_size >> miss_num >> attr_type;
			meta_file.ignore( 1000, '\t' );
			getline( meta_file, m_vecMeta[i].m_strName, '\n' );
			if (meta_file.fail() == true)
			{
				temp_sstream << "Microdata::ReadFiles Error: Metadata for attribute " << i + 1 << " has an incorrect format";
				temp_exception.SetMsg( temp_sstream.str() );
				throw( temp_exception );
			}
			if (domain_size < 1)
			{
				temp_sstream << "Microdata::ReadFiles Error: Incorrect domain size for attribute " << i + 1;
				temp_exception.SetMsg( temp_sstream.str() );
				throw( temp_exception );
			}
			if (miss_num < 0 || miss_num > domain_size)
			{
				temp_sstream << "Microdata::ReadFiles Error: Number of missing values for attribute " << i + 1 << " is smaller than 0 or larger than domain size";
				temp_exception.SetMsg( temp_sstream.str() );
				throw( temp_exception );
			}
			
			Metadata& r_meta = m_vecMeta[i];
			r_meta.m_nType = anony::AttrType( attr_type );
			r_meta.m_nMinValue.SetType( r_meta.m_nType );
			r_meta.m_nMaxValue.SetType( r_meta.m_nType );
			temp_unit.SetType( r_meta.m_nType );
			switch( attr_type )
			{
			case anony::INTEGER :
				{
					long temp_long;
					meta_file >> r_meta.m_nMinValue.l >> r_meta.m_nMaxValue.l;
					if (r_meta.m_nMinValue.l >= r_meta.m_nMaxValue.l || meta_file.fail() == true)
					{
						temp_sstream << "Microdata::ReadFiles Error: Incorrect MIN or MAX value for attribute " << i + 1;

						temp_exception.SetMsg( temp_sstream.str() );
						throw( temp_exception );
					}
					for (j = 0; j < miss_num; ++j)
					{
						meta_file >> temp_long;
						if (meta_file.fail() == true)
						{
							temp_sstream << "Microdata::ReadFiles Error: Fail to read missing value " << j + 1 << " of attribute " << i + 1;
							temp_exception.SetMsg( temp_sstream.str() );
							throw( temp_exception );
						}
						temp_unit.l = temp_long;
						r_meta.m_vecMissValue.push_back( temp_unit );
					}
					break;
				}

			case anony::CATEGORICAL :
				{
					m_vecMeta[i].m_nMinValue.l = numeric_limits<long>::max();
					m_vecMeta[i].m_nMaxValue.l = numeric_limits<long>::min();
					r_meta.m_nMinValue.SetType( anony::CATEGORICAL );
					r_meta.m_nMaxValue.SetType( anony::CATEGORICAL );
					
					pair<long, string> temp_map;
					pair<tr1::unordered_map<long, string>::iterator, bool> rslt_pair;
					for (j = 0; j < domain_size; ++j)
					{
						meta_file >> temp_map.first;
						if (meta_file.fail() == true)
						{
							temp_sstream << "Microdata::ReadFiles Error: Fail to read value " << j + 1 << " of attribute " << i + 1;
							temp_exception.SetMsg( temp_sstream.str() );
							throw( temp_exception );
						}

						m_vecMeta[i].m_nMinValue.l = min( temp_map.first, m_vecMeta[i].m_nMinValue.l );
						m_vecMeta[i].m_nMaxValue.l = max( temp_map.first, m_vecMeta[i].m_nMaxValue.l );

						meta_file.ignore( 1000, '\t' );
						getline( meta_file, temp_map.second, '\n' );
						rslt_pair = r_meta.m_mapValueName.insert( temp_map );
						if (rslt_pair.second == false)
						{
							temp_sstream << "Microdata::ReadFiles Error: Value " << j + 1 << " of attribute " << i + 1 << " is a duplicate";
							temp_exception.SetMsg( temp_sstream.str() );
							throw( temp_exception );
						}
						if (j >= domain_size - miss_num)
						{
							temp_unit.l = temp_map.first;
							r_meta.m_vecMissValue.push_back( temp_unit );
						}
					}
					break;
				}

			case anony::FLOAT :
				{
					float temp_float;
					meta_file >> r_meta.m_nMinValue.f >> r_meta.m_nMaxValue.f;
					if (r_meta.m_nMinValue.f >= r_meta.m_nMaxValue.f || meta_file.fail() == true)
					{
						temp_sstream << "Microdata::ReadFiles Error: Incorrect MIN or MAX value for attribute " << i + 1;
						temp_exception.SetMsg( temp_sstream.str() );
						throw( temp_exception );
					}
					
					for (j = 0; j < miss_num; ++j)
					{
						meta_file >> temp_float;
						if (meta_file.fail() == true)
						{
							temp_sstream << "Microdata::ReadFiles Error: Fail to read missing value " << j + 1 << " of attribute " << i + 1;
							temp_exception.SetMsg( temp_sstream.str() );
							throw( temp_exception );
						}
						temp_unit.f = temp_float;
						r_meta.m_vecMissValue.push_back( temp_unit );
					}
					break;
				}
			default:
				{
					temp_sstream << "Microdata::ReadFiles Error: The type of attribute " << i + 1 << " is incorrect";
					temp_exception.SetMsg( temp_sstream.str() );
					throw( temp_exception );
				}
			}
		}
		
		meta_file.close();

		//	Initialize the vector of tuple values
		m_vecTuple.insert( m_vecTuple.end(), attr_num * tuple_num, UnitValue() );
		long temp_long;
		
		//	Read the tuple values
		for (i = 0; i < tuple_num; ++i)
		{
			micro_file >> temp_long;
			for (j = 0; j < attr_num; ++j)
			{
				UnitValue& r_value = m_vecTuple[i * attr_num + j];
				r_value.SetType( m_vecMeta[j].m_nType );
				switch( m_vecMeta[j].m_nType )
				{
				case anony::INTEGER :
					{						
						micro_file >> r_value.l;
						if (micro_file.fail() == true)
						{
							temp_sstream << "Microdata::ReadFiles Error: Fail to read value " << j + 1 << " of tuple " << i + 1;
							temp_exception.SetMsg( temp_sstream.str() );
							throw( temp_exception );
						}
						if (r_value.l < m_vecMeta[j].m_nMinValue.l
							|| r_value.l > m_vecMeta[j].m_nMaxValue.l)
						{
							temp_sstream << "Microdata::ReadFiles Error: Value " << j + 1 << " of tuple " << i + 1 << " is not defined in the metadata";
							temp_exception.SetMsg( temp_sstream.str() );
							throw( temp_exception );
						}
						break;
					}

				case anony::CATEGORICAL :
					{
						micro_file >> m_vecTuple[i * attr_num + j].l;
						if (micro_file.fail() == true)
						{
							temp_sstream << "Microdata::ReadFiles Error: Fail to read value " << j + 1 << " of tuple " << i + 1;
							temp_exception.SetMsg( temp_sstream.str() );
							throw( temp_exception );
						}
						if (m_vecMeta[j].m_mapValueName.find( m_vecTuple[i * attr_num + j].l ) == m_vecMeta[j].m_mapValueName.end())
						{
							temp_sstream << "Microdata::ReadFiles Error: Value " << j + 1 << " of tuple " << i + 1 << " is not defined in the metadata";
							temp_exception.SetMsg( temp_sstream.str() );
							throw( temp_exception );
						}
						break;
					}

				case anony::FLOAT :
					{
						micro_file >> m_vecTuple[i * attr_num + j].f;
						if (micro_file.fail() == true)
						{
							temp_sstream << "Microdata::ReadFiles Error: Fail to read value " << j + 1 << " of tuple " << i + 1;
							temp_exception.SetMsg( temp_sstream.str() );
							throw( temp_exception );
						}
						if (m_vecTuple[i * attr_num + j].f < m_vecMeta[j].m_nMinValue.f
							|| m_vecTuple[i * attr_num + j].f > m_vecMeta[j].m_nMaxValue.f)
						{
							temp_sstream << "Microdata::ReadFiles Error: Value " << j + 1 << " of tuple " << i + 1 << " is not defined in the metadata";
							temp_exception.SetMsg( temp_sstream.str() );
							throw( temp_exception );
						}
						break;
					}
				}
			}
			if (micro_file.fail() == true)
			{
				temp_sstream << "Microdata::ReadFiles Error: Fail to read the values of tuple " << i + 1;
				temp_exception.SetMsg( temp_sstream.str() );
				throw( temp_exception );
			}
		}

		micro_file.close();
	}
	catch( AnonyException e )
	{
		if (micro_file.is_open() == true)
		{
			micro_file.close();
		}
		if (meta_file.is_open() == true)
		{
			meta_file.close();
		}
		m_vecTuple.clear();
		m_vecMeta.clear();

		cout << e.GetMsg() << endl;

		throw( e );
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////


const QString& Microdata::GetValueQstr( long nTupleIdx, long nAttrIdx )
{
	if ((nTupleIdx < 0) || (nTupleIdx >= m_nTupleNum) || (nAttrIdx < 0) || (nAttrIdx >= m_nAttrNum))
	{
		AnonyException temp_exception;
		temp_exception.m_bShallContinue = true;
		temp_exception.SetMsg( "Microdata::GetValue Error: Incorrect value index" );
		throw( temp_exception );
	}

	UnitValue& r_value = m_vecTuple[nTupleIdx * m_nAttrNum + nAttrIdx];
	m_qstrTemp.clear();
	switch( r_value.GetType() )
	{
	case anony::INTEGER :
		{
			m_qstrTemp.setNum( r_value.l );
			break;
		}

	case anony::FLOAT :
		{
			m_qstrTemp.setNum( r_value.f );
			break;
		}

	case anony::CATEGORICAL :
		{
			m_qstrTemp = (*(m_vecMeta[nAttrIdx].m_mapValueName.find( r_value.l ))).second.c_str();
			break;
		}
	}

	return m_qstrTemp;
}


//////////////////////////////////////////////////////////////////////////


void Microdata::SortByAttr( long nAttr, vector<long>& rVecId, bool bAscend )
{
	long i;
	ValueIdPair temp_pair;

	vector<ValueIdPair> vec_pair;
	vec_pair.insert( vec_pair.end(), m_nTupleNum, temp_pair );

	for (i = 0; i < m_nTupleNum; ++i)
	{
		vec_pair[i].m_nTplId = i;
		vec_pair[i].m_uValue = m_vecTuple[i * m_nAttrNum + nAttr];
	}

	if (bAscend == true)
	{
		sort( vec_pair.begin(), vec_pair.end() );
	}
	else
	{
		sort( vec_pair.begin(), vec_pair.end(), greater<ValueIdPair>() );
	}

	rVecId.clear();
	rVecId.reserve( m_nTupleNum );
	for (i = 0; i < m_nTupleNum; ++i)
	{
		rVecId.push_back( vec_pair[i].m_nTplId );
	}

	return;
}
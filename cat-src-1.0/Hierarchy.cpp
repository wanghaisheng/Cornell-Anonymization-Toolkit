#include "stdafx.h"
#include "Hierarchy.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "AnonyException.h"

#pragma warning(disable : 4786)


//////////////////////////////////////////////////////////////////////////


bool Hierarchy::Init( const char* pFileName, const long nLeafNum, const anony::AttrType nAttrType )
{
	AnonyException temp_exception;
	temp_exception.m_bShallContinue = true;
	stringstream temp_sstream;

	if (nAttrType != anony::INTEGER && nAttrType != anony::CATEGORICAL && nAttrType != anony::FLOAT)
	{
		temp_sstream << "Hierarchy::Init Error: Incorrect attribute type";
		temp_exception.SetMsg( temp_sstream.str() );
		throw( temp_exception );
	}
	m_nAttrType = nAttrType;

	ifstream in_file( pFileName );

	try
	{
		if (in_file.is_open() == false)
		{
			temp_sstream << "Hierarchy::Init Error: Cannot Open Input File " << pFileName;
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
			return false;
		}

		HrchNode temp_node;	
		long level_num, node_num;
		long i, j, k;
		long child_pos, parent_pos;
		string temp_str;


		//	Read the number of levels in the hierarchy (excluding the leaf level)
		in_file >> level_num;
		getline( in_file, temp_str, '\n' );
		if (level_num < 1 || level_num > nLeafNum || in_file.fail() == true)
		{
			temp_sstream << "Hierarchy::Init Error: The number of levels is incorrect";
			temp_exception.SetMsg( temp_sstream.str() );
			throw( temp_exception );
		}

		//	Read the nodes in each level
		temp_node.m_nChildLeft = temp_node.m_nChildRight = temp_node.m_nParent = -1;
		for (i = 0; i < level_num; ++i)
		{
			in_file >> node_num;
			if (node_num < 1 || node_num > nLeafNum || in_file.fail() == true)
			{
				temp_sstream << "Hierarchy::Init Error: The number of nodes at level " <<  i + 1 << " is incorrect";
				temp_exception.SetMsg( temp_sstream.str() );
				throw( temp_exception );
			}
			if ((i == level_num - 1) && (node_num != 1))
			{
				temp_sstream << "Hierarchy::Init Error: The highest level should has only one node";
				temp_exception.SetMsg( temp_sstream.str() );
				throw( temp_exception );
			}
			getline( in_file, temp_str, '\n' );

			m_vecNode.reserve( m_vecNode.size() + node_num );
			m_vecLevel.push_back( m_vecNode.size() );
			m_vecCount.push_back( node_num );

			for (j = 0; j < node_num; ++j)
			{
				if (nAttrType == anony::FLOAT)
				{
					in_file >> temp_node.m_uInterval.m_uLeft.f >> temp_node.m_uInterval.m_uRight.f;
				}
				else
				{
					in_file >> temp_node.m_uInterval.m_uLeft.l >> temp_node.m_uInterval.m_uRight.l;
				}
				
				if (in_file.fail() == true)
				{
					temp_sstream << "Hierarchy::Init Error: The interval representation of node " << j + 1 << " at level " << i + 1 << " is incorrect";
					temp_exception.SetMsg( temp_sstream.str() );
					throw( temp_exception );
				}
				in_file.ignore( 999, '\t' );
				getline( in_file, temp_node.m_strName, '\n' );
				m_vecNode.push_back( temp_node );
			}

			if (i > 0)
			{
				child_pos = m_vecLevel[i - 1];
				k = m_vecNode.size();
				for (parent_pos = m_vecLevel[i]; parent_pos < k; ++parent_pos)
				{
					m_vecNode[parent_pos].m_nChildLeft = child_pos;
					while (m_vecNode[child_pos].m_uInterval.m_uRight < m_vecNode[parent_pos].m_uInterval.m_uRight)
					{
						m_vecNode[child_pos].m_nParent = parent_pos;
						++child_pos;
					}
					if (m_vecNode[child_pos].m_uInterval.m_uRight != m_vecNode[parent_pos].m_uInterval.m_uRight)
					{
						if (nAttrType == anony::FLOAT)
						{
							temp_sstream << "Hierarchy::Init Error: While processing " << pFileName << ": The intermediate node " << m_vecNode[parent_pos].m_uInterval.m_uLeft.f << "-"
								<< m_vecNode[parent_pos].m_uInterval.m_uRight.f << " at level " << i << " partially overlaps with the nodes in its subtree";
						}
						else
						{
							temp_sstream << "Hierarchy::Init Error: While processing " << pFileName << ": The intermediate node " << m_vecNode[parent_pos].m_uInterval.m_uLeft.l << "-"
								<< m_vecNode[parent_pos].m_uInterval.m_uRight.l << " at level " << i << " partially overlaps with the nodes in its subtree";
						}
						temp_exception.SetMsg( temp_sstream.str() );
						throw( temp_exception );
					}
					else
					{
						m_vecNode[child_pos].m_nParent = parent_pos;
						m_vecNode[parent_pos].m_nChildRight = child_pos;
						++child_pos;
					}
				}
			}
		}
	}
	catch( AnonyException e )
	{
		cout << e.GetMsg() << endl;
		m_vecNode.clear();
		m_vecLevel.clear();
		m_vecCount.clear();
		in_file.close();
		throw( e );
	}	

	in_file.close();
	return true;
}
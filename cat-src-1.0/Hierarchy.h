#ifndef HIERARCHY_H_20081218
#define HIERARCHY_H_20081218

#include "UnitValue.h"

#include <vector>
#include <string>

#pragma warning(disable : 4786)

using namespace std;


//////////////////////////////////////////////////////////////////////////


class HrchNode		//	The class for representing a node in a generalization hierarchy
{
public:
	HrchNode(){};

	HrchNode( const HrchNode& rOtherNode )
	{
		m_uInterval = rOtherNode.m_uInterval;
		m_nChildLeft = rOtherNode.m_nChildLeft;
		m_nChildRight = rOtherNode.m_nChildRight;
		m_nParent = rOtherNode.m_nParent;
		m_strName = rOtherNode.m_strName;
	}

	const HrchNode& operator = ( const HrchNode& rOtherNode )
	{
		m_uInterval = rOtherNode.m_uInterval;
		m_nChildLeft = rOtherNode.m_nChildLeft;
		m_nChildRight = rOtherNode.m_nChildRight;
		m_nParent = rOtherNode.m_nParent;
		m_strName = rOtherNode.m_strName;
		return *this;
	}

	bool operator < ( const HrchNode& rOtherNode ) const
	{
		return m_uInterval.m_uRight < rOtherNode.m_uInterval.m_uRight;
	}

public:
	UnitIntvl	m_uInterval;	//	The left (right) endpoint of the interval corresponds to the minimum (maximum) leaf values in the subtree of the node

	long m_nChildLeft;			//	The position of the leftmost child-node in the node vector (see class Hierarchy)
	long m_nChildRight;			//	The position of the rightmost child-node in the node vector (see class Hierarchy)
	long m_nParent;				//	The position of the parent node in the node vector (see class Hierarchy)

	string m_strName;			//	The name of the node
};


//////////////////////////////////////////////////////////////////////////


class Hierarchy		//	The class for representing the generalization hierarchy of an attribute
{
public:

	bool Init( const char* pFileName, const long nLeafNum, const anony::AttrType nAttrType );	//	Read the hierarchy from a file

	const Hierarchy& operator = ( const Hierarchy& rHierarchy )
	{
		m_vecNode = rHierarchy.m_vecNode;
		return *this;
	}

	const HrchNode & GetNode( long nNodeIdx )
	{
		return m_vecNode[nNodeIdx];
	}

	const vector<HrchNode>& GetVecNode()
	{
		return m_vecNode;
	}

	const vector<long>& GetVecLevel()
	{
		return m_vecLevel;
	}

	anony::AttrType GetType()
	{

		return m_nAttrType;
	}

	long GetCount( long nAttr )
	{
		return m_vecCount[nAttr];
	}

private:
	vector<HrchNode>	m_vecNode;	//	A vector storing the nodes in the hierarchy
	vector<long>	m_vecLevel;		//	The i-th value in the vector stores the position of the first i-th level nodes (the 0-th level is one level above the leaves)
	vector<long>	m_vecCount;		//	The i-th value in the vector stores the number of i-th level nodes
	anony::AttrType	m_nAttrType;	//	The type of the attribute to which the hierarchy corresponds
};

#endif // HIERARCHY_H_20081218
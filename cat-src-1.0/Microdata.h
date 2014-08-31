#ifndef MICRODATA_H_20081218
#define MICRODATA_H_20081218

#include "UnitValue.h"
#include <vector>
#include <map>
#include <string>
#include <unordered_map>
#include <sstream>
//#include <qstring>

using namespace std;



//////////////////////////////////////////////////////////////////////////


class Metadata		//	The class for representing the metadata of an attribute
{
public:
	string	m_strName;			//	The name of the attribute
	anony::AttrType	m_nType;	//	The type of the attribute (integer, categorical, or fractional)
	vector<UnitValue>	m_vecMissValue;		//	The integer codes that represent the missing values of the attributes
	tr1::unordered_map<long,string>	m_mapValueName;	//	For categorical attributes only: the mapping from an integer code to its corresponding string name
	UnitValue	m_nMinValue;	//	For numeric attributes only: the minimum value of the attribute
	UnitValue	m_nMaxValue;	//	For numeric attributes only: the maximum value of the attribute
};


//////////////////////////////////////////////////////////////////////////


class ValueIdPair	//	The class for representing a pair of <value, tuple ID>. Used to obtain a sorted order of the tuples according to their certain values.
{
public:
	bool operator < (const ValueIdPair& rOther) const
	{
		return m_uValue < rOther.m_uValue;
	}

	bool operator > (const ValueIdPair& rOther) const
	{
		return m_uValue > rOther.m_uValue;
	}

public:
	UnitValue	m_uValue;	//	Certain value of a tuple
	long	m_nTplId;		//	The ID of a tuple
};


//////////////////////////////////////////////////////////////////////////


class Microdata		//	The class for representing the microdata
{
public:
	bool ReadFiles( const char* pMicrodata, const char* pMetadata );	//	Reads the microdata as well as the correpsonding metadata

	const UnitValue& GetValue( long nTupleIdx, long nAttrIdx ) const	//	Get the value of tuple nTupleIdx on attribute nAttrIdx
	{
		return m_vecTuple[nTupleIdx * m_nAttrNum + nAttrIdx];
	}

	const QString& GetValueQstr( long nTupleIdx, long nAttrIdx );		//	Get the string representation of the value of tuple nTupleIdx on attribute nAttrIdx

	const char* GetAttrName( long nAttrIdx )							//	Get the name of attribute nAttrIdx
	{
		return m_vecMeta[nAttrIdx].m_strName.c_str();
	}

	const vector<UnitValue>& GetVecTuple() const	//	Get a reference to the vector storing the tuples
	{
		return m_vecTuple;
	}

	const vector<Metadata>& GetVecMeta() const		//	Get a reference to the vector storing the metadata					
	{
		return m_vecMeta;
	}

	const long GetTupleNum() const		//	Get the number of tuples
	{
		return m_nTupleNum;
	}

	const long GetAttrNum() const		//	Get the number of attributes
	{
		return m_nAttrNum;
	}

	void SortByAttr( long nAttr, vector<long>& rVecId, bool bAscend = true );	//	Sort the tuples by attribute nAttr, and store the ordering in rVecId

private:
	vector<UnitValue>	m_vecTuple;		//	A vector storing the value of the tuples. The i-th value of the j-th tuple is stored at the j * d + i position, where d is the number of attributes.
	vector<Metadata>	m_vecMeta;		//	A vector storing the metadata of the each attribute
	long	m_nTupleNum;				//	The number of tuples
	long	m_nAttrNum;					//	The number of attributes;
	stringstream m_strmTemp;			//	Used in GetValueQStr
	QString m_qstrTemp;					//	Used in GetValueQStr
};

#endif // MICRODATA_H_20081218
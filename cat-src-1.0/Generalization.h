#ifndef GENERALIZATION_H_20081218
#define GENERALIZATION_H_20081218

#include "UnitValue.h"
#include "Microdata.h"
#include "Hierarchy.h"
#include "AnonyException.h"
#include <vector>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <set>


class Generalization;

//////////////////////////////////////////////////////////////////////////


class AttrWghtPair		//	A pair of <Attribute, Attribute Weight>, used to specify the weight of an attribute in the generalization function
{
public:
	long	m_nAttr;
	float	m_fWght;
};


//////////////////////////////////////////////////////////////////////////


class CompareByAttr		//	A functor used to sort a vector of AttrWghtPair by the order of the attributes
{
public:
	bool operator() ( const AttrWghtPair& rFirst, const AttrWghtPair& rSecond )
	{
		return rFirst.m_nAttr < rSecond.m_nAttr;
	}
};


//////////////////////////////////////////////////////////////////////////


class CompareByWght		//	A functor used to sort a vector of AttrWghtPair by the weights of the attributes
{
public:
	bool operator() ( const AttrWghtPair& rFirst, const AttrWghtPair& rSecond )
	{
		return rFirst.m_fWght > rSecond.m_fWght;
	}
};


//////////////////////////////////////////////////////////////////////////


class EqvClass		//	The class for representing an equivalence class
{
public:
	EqvClass()
	{
	}

	EqvClass( const EqvClass& rEqvClass )
	{
		m_lstTplId = rEqvClass.m_lstTplId;
		m_mapSenCnt = rEqvClass.m_mapSenCnt;
		m_vecNodePos = rEqvClass.m_vecNodePos;
	}

	const EqvClass& operator = ( const EqvClass& rEqvClass )
	{
		m_lstTplId = rEqvClass.m_lstTplId;
		m_mapSenCnt = rEqvClass.m_mapSenCnt;
		m_vecNodePos = rEqvClass.m_vecNodePos;
		return *this;
	}

	void Clear()
	{
		m_lstTplId.clear();
		m_vecNodePos.clear();
		m_mapSenCnt.clear();
	}	

public:
	list<long>	m_lstTplId;					//	A list that stores the ID of the tuples in the equivalence class

	vector<long>	m_vecNodePos;			//	A vector that captures the generalized values of the equivalence class on each QI attribute. 
											//	The i-th value stores the position of the node in the vector of the generalization hierarchy of the i-th QI attribute.
											//	(Recall that each generalized value corresponds to a node in the generalization hierarchy.)
											//	A -1 indicates that the attribute has not been generalized.

	map<UnitValue, long>	m_mapSenCnt;	//	A mapping between the sensitive values and their frequencies in the equivalence class
};


//////////////////////////////////////////////////////////////////////////


class RiskIdPair	//	A pair of <tuple ID, risk>, used to sort the tuples by their diclosure risks
{
public:
	bool operator < (const RiskIdPair& rOther) const
	{
		return m_fRisk < rOther.m_fRisk;
	}

	bool operator > (const RiskIdPair& rOther) const
	{
		return m_fRisk > rOther.m_fRisk;
	}

public:
	float	m_fRisk;
	long	m_nTplId;
};


//////////////////////////////////////////////////////////////////////////


class Verifier		//	An interface for functors that decide whether the risks of an equivalence class is too large
{
public:
	virtual bool Verify( const EqvClass& rEqvCls ) = 0;

	virtual bool CheckBeforeAnonymize( const Generalization& rGen ) = 0;

	virtual bool SetRisk( const vector<EqvClass*>& rVecPtEqv, vector<float>& rVecRisk ) = 0;	

	virtual bool CheckBeforeEvalRisk( const Generalization& rGen ) = 0;
};


//////////////////////////////////////////////////////////////////////////


class L_Diversity : public Verifier	//	Evaluates whether an equivalence class is l-diverse
{
public:
	L_Diversity()
	{
		m_nL = 0;
		m_fC = 0.0f;
	}

	L_Diversity( const long l, const float c)
	{
		m_nL = l;
		m_fC = c;
	}

	virtual bool Verify( const EqvClass& rEqvCls );

	virtual bool CheckBeforeAnonymize( const Generalization& rGen );

	virtual bool SetRisk( const vector<EqvClass*>& rVecPtEqv, vector<float>& rVecRisk );

	virtual bool CheckBeforeEvalRisk( const Generalization& rGen );

public:
	long	m_nL;
	float	m_fC;	
	long	m_nBckKnw;

	map<UnitValue, long>	m_mapSenCnt;
	vector<long>	m_vecSenCnt;
};


//////////////////////////////////////////////////////////////////////////

class T_Closeness : public Verifier // Evaluates whether an equivalence class is t-closeness
{
public:

	T_Closeness()
	{
		m_fT = 0.0f;
	}

	T_Closeness( const float t, const Generalization& rGen );

	virtual bool Verify( const EqvClass& rEqvCls );

	virtual bool CheckBeforeAnonymize( const Generalization& rGen );

	virtual bool SetRisk( const vector<EqvClass*>& rVecPtEqv, vector<float>& rVecRisk );

	virtual bool CheckBeforeEvalRisk( const Generalization& rGen );

	// compute the Earth Move Distance
	float EMD( long numTable, long numEqCls, vector<UnitValue>& vecId, map<UnitValue, long>& mapTable, const map<UnitValue, long>& mapEqCls );

public:
	float					m_fT;
	long					m_nTuples;
	anony::AttrType			m_nSenType;
	vector<UnitValue>		m_vecSenId;
	map<UnitValue, long>	m_mapSenDistn;
};


//////////////////////////////////////////////////////////////////////////


class Generalization		//	The class for representing the generalization of a microdata table
{
public:
	Generalization()
	{
		m_pMicrodata = NULL;
		m_nSenAttr = -1;
		m_bRootDirty = false;
		m_bGenExists = false;
	}

	void Clear()
	{
		m_pMicrodata = NULL;
		m_nSenAttr = -1;
		m_vecHrch.clear();
		m_vecQiAttr.clear();
		m_vecGenLevel.clear();
		m_vecCantGen.clear();
		m_setDeleted.clear();
		m_lstEqvClass.clear();
		m_eqvRootClass.Clear();
		m_vecQiInd.clear();
		m_vecPtEqv.clear();
		m_vecStatus.clear();
		m_bRootDirty = false;
		m_bGenExists = false;
	}	

	bool Init( Microdata* pMicrodata, const vector<string>& rVecHrchFile, const vector<long>& rVecQi, const long nSenAttr );	
	//	Specify the microdata table, the QI and sensitive attributes, as well as the hierarchies of the QI attributes

	bool SetWeights( const vector<float>& rVecWeights );	//	Specify the weights of each attribute (by default all attributes have the same weights)

	bool Anonymize( Verifier& rVerifier );	//	Anonymize the microdata, based on the criterion specified by rVerifier

	bool EvaluateRisk( const vector<long>& rVecAttr, Verifier& rVerifier );	//	Calculate the risk of each tuple, based on the attributes specified in rVecAttr and the criterion specified by rVerifier

	void EvaluateRisk( const vector<long>& rVecAttr, Verifier& rVerifier, long nBinNum, vector<long>& rVecCnt, float& fMaxRisk, float fMinRisk, bool bNeedsUpdate = true );
	//	Calculate the risk of each tuples, generate a histogram of risks with nBinNum bins, and put the bin counts in rVecCnt
	//	bNeedsUpdate specifies whether the risk of any tuple has to be re-computed after the last evaluation


	const QString& GetValueQstr( long nTupleIdx, long nAttrIdx );	//	Get the string representation of the value of tuple nTupleIdx on attribute nAttrIdx

	bool GetJointDensity( const long nAttrX, const long nAttrY, long& nWidth, long& nHeight, vector<float>& rVecOri, vector<float>& rVecAnony, vector<QString>& rVecNameX, vector<QString>& rVecNameY, float& fheightMax );
	//	Generate two contingency tables where the x-dimension is nAttrX and the y-dimension is nAttrY
	//	nWidth and nHeight capture the number of values in the x- and y-dimensions, respectively
	//	rVecOri and rVecAnony are the contingency tables before and after generalization, respectively
	//	rVecNameX and rVecNameY are the vectors of the string representations for the x- and y-dimensions, respectively

	bool GetMarginalDensity( const long nAttr, long& nLength, vector<float>& rVecOri, vector<float>& rVecAnony, vector<QString>& rVecName );
	//	Generate two vectors where the dimension is nAttr
	//	nLength capture the number of values in the dimension
	//	rVecOri and rVecAnony are the contingency tables before and after generalization, respectively

	void Delete( const float fThreshold );	//	Delete all tuples whose risks is larger than fThreshold

	void DelTopRisk( const long nTopK );	//	Delete the tuples with the top-k risks

	void Clean( vector<long>& rVecId );		//  Shrink the order vector after deletion/sorting

	void Resume( vector<long>& rVecId );	//  Resume the order vector after recovering

	void RecoverAll();						//	Recover all deleted tuples

	void SortByRisk( vector<long>& rVecId, bool bAscend = true );	//	Sort the tuples by their risks

protected:
	bool InitRootClass();	//	Initialize the "root" equivalence class that contains all tuples in the microdata

public:
	Microdata*	m_pMicrodata;		//	A pointer to the microdata
	vector<Hierarchy>	m_vecHrch;	//	The vector that stores the hierarchies of the QI attributes

	long	m_nSenAttr;				//	Specifies attribute m_nSenAttr as the sensitive attribute
	vector<long>	m_vecGenLevel;	//	The i-th value in the vector indicates the level to which the i-th attribute has been generalized to. 
									//	A 0 indicates that no generalization has been performed on the attribute.

	vector<AttrWghtPair>	m_vecQiAttr;	//	The i-th element in the specifies the i-th QI attribute and its weight
	vector<long>	m_vecQiInd;		//	If the i-th attribute in the microdata is the j-th QI attribute, the i-th value in this vector equals j. 
									//	If the i-th attribute is not a QI-attribute, the i-th value in this vector equals -1.
	
	EqvClass	m_eqvRootClass;		//	The "root" equivalence that contains all tuples in the microdata
	list<EqvClass>	m_lstEqvClass;	//	The list of equivalence in the generalized table
	set<long>	m_setDeleted;		//	A set that stores the ID of the deleted tuples
	vector<long>	m_vecCantGen;	//	A set that stores the ID of the tuples that cannot be generalized (e.g., tuples that have a missing value on some QI attribute)
	vector<float>	m_vecRisk;		//	The i-th value in this vector stores the risk of the i-th tuple
	vector<EqvClass*>	m_vecPtEqv;	//	The i-th pointer in this vector points to the equivalence class that contains the i-th tuple
	vector<anony::TplStatus>	m_vecStatus;	//	The i-th value in this vector indicates the status of the i-th tuple (e.g., whether or not the tuple has been deleted)

	bool m_bRootDirty;		//	Indicate whether the "root" equivalence class need to be updated (e.g., when some tuples has been deleted after the last update)
	bool m_bGenExists;		//	Indicate whether a generalization exists

	stringstream m_strmTemp;	//	Used in GetValueQstr
	QString	m_qstrTemp;			//	Used in GetValueQstr
};

#endif // GENERALIZATION_H_20081218
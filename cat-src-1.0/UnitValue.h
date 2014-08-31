#ifndef UNITVALUE_H_20081218
#define UNITVALUE_H_20081218

#include <QString>

namespace anony
{
	enum AttrType
	{
		INTEGER = 0, CATEGORICAL = 1, FLOAT = 2
	};

	enum TplStatus
	{
		CANTGEN = 0, DELETED = 1, NORMAL = 2
	};

	enum LogType
	{
		GENERALIZATION = 0, DELETE = 1, EVALUATE = 2
	};

	enum StateType
	{
		INIT = 0, AFTER_DATA = 1, AFTER_HIERARCHY = 2, AFTER_GENERALIZATION = 3, AFTER_RISK = 4, AFTER_DELETE = 5
	};

	enum VerifierType
	{
		DIVERSITY = 0, CLOSENESS = 1
	};
}


//////////////////////////////////////////////////////////////////////////

class UnitValue		//	The class for representing each value of a tuple;
{
public:
	UnitValue()
	{
		m_nUnitType = anony::INTEGER;
	}

	UnitValue( anony::AttrType uType )
	{
		m_nUnitType = uType;
	}

	UnitValue( const UnitValue& rOtherValue )
	{
		l = rOtherValue.l;
		m_nUnitType = rOtherValue.GetType();
	}

	void SetType( anony::AttrType uType )
	{
		m_nUnitType = uType;
	}

	anony::AttrType GetType() const
	{
		return m_nUnitType;
	}

	const UnitValue& operator = ( const long nValue )
	{
		l = nValue;
		m_nUnitType = anony::INTEGER;
		return *this;
	}

	const UnitValue& operator = ( const float fValue )
	{
		f = fValue;
		m_nUnitType = anony::FLOAT;
		return *this;
	}

	const UnitValue& operator = ( const UnitValue& rOtherValue )
	{
		l = rOtherValue.l;
		m_nUnitType = rOtherValue.GetType();
		return *this;
	}
	
	bool operator < ( const UnitValue& rOtherValue ) const
	{
		if (m_nUnitType == anony::FLOAT)
		{
			return f < rOtherValue.f;
		}
		else
		{
			return l < rOtherValue.l;
		}
	}

	bool operator == ( const UnitValue& rOtherValue ) const
	{
		if (m_nUnitType == anony::FLOAT)
		{
			return f == rOtherValue.f;
		}
		else
		{
			return l == rOtherValue.l;
		}
	}

	bool operator == ( long lValue ) const
	{
		return (l == lValue);
	}

	bool operator != ( const UnitValue& rOtherValue ) const
	{
		if (m_nUnitType == anony::FLOAT)
		{
			return f != rOtherValue.f;
		}
		else
		{
			return l != rOtherValue.l;
		}
	}

	bool operator > ( const UnitValue& rOtherValue ) const
	{
		if (m_nUnitType == anony::FLOAT)
		{
			return f > rOtherValue.f;
		}
		else
		{
			return l > rOtherValue.l;
		}
	}

public:
	union
	{
		long l;
		float f;
	};

private:
	anony::AttrType	m_nUnitType;	//	Each value can be an integer, a categorical value (represented using an integer), or a float number.
};


//////////////////////////////////////////////////////////////////////////


class UnitIntvl		//	The class for representing an interval
{
public:
	bool IsCoveredBy( const UnitIntvl& rIntvl ) const
	{
		return !(m_uLeft < rIntvl.m_uLeft || m_uRight > rIntvl.m_uRight);
	}

	bool Covers( const UnitValue& rValue ) const
	{
		if (rValue.GetType() == anony::FLOAT)
		{
			return (rValue.f <= m_uRight.f && rValue.f >= m_uLeft.f);
		}
		else
		{
			return (rValue.l <= m_uRight.l && rValue.l >= m_uLeft.l);
		}
	}

public:
	UnitValue m_uLeft;
	UnitValue m_uRight;
};


//////////////////////////////////////////////////////////////////////////

/*
class QString
{
public:
	void clear()
	{
	}

	void setNum( long nValue )
	{
	}

	void setNum( float fValue )
	{
	}

	const QString& operator = ( const char* )
	{
		return *this;
	}

};
*/

#endif // UNITVALUE_H_20081218
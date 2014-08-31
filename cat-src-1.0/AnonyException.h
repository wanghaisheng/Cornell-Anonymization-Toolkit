#ifndef ANONYEXCEPTION_H_20081218
#define ANONYEXCEPTION_H_20081218

#include <exception>
#include <string>

using namespace std;

class AnonyException : public exception		//	The class for representing exceptions
{
public:
	AnonyException()
	{
		m_bShallContinue = false;
	}

	AnonyException( const AnonyException& rExcep )
	{
		m_strMsg = rExcep.m_strMsg;
		m_bShallContinue = rExcep.m_bShallContinue;
	}

	const AnonyException& operator = ( const AnonyException& rExcep )
	{
		m_strMsg = rExcep.m_strMsg;
		m_bShallContinue = rExcep.m_bShallContinue;
	}

	const string& GetMsg()
	{
		return m_strMsg;
	}

	void SetMsg( const string& rStrMsg )
	{
		m_strMsg = rStrMsg;
	}

public:
	string	m_strMsg;				//	The error message
	bool	m_bShallContinue;		//	Whether the program should continue or terminate
};

#endif // ANONYEXCEPTION_H_20081218
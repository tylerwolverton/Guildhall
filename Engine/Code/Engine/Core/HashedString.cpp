#include "Engine/Core/HashedString.hpp"
#include "Engine/Core/HashUtils.hpp"


//-----------------------------------------------------------------------------------------------
std::map<uint32_t, std::string> HashedString::s_stringTable;


//-----------------------------------------------------------------------------------------------
HashedString::HashedString( const char* input )
{
	m_id = Hash( (byte*)input, strlen( input ) );
	AddToStringTable( input );
}


//-----------------------------------------------------------------------------------------------
HashedString::HashedString( const std::string& input )
{
	m_id = Hash( (byte*)input.c_str(), input.size() );
	AddToStringTable( input );
}


//-----------------------------------------------------------------------------------------------
HashedString::HashedString( const HashedString& other )
{
	m_id = other.m_id;
}


//-----------------------------------------------------------------------------------------------
HashedString::HashedString( const HashedString&& other )
{
	if ( this == &other )
	{
		return;
	}

	m_id = other.m_id;
}


//-----------------------------------------------------------------------------------------------
HashedString& HashedString::operator=( const HashedString&& other )
{
	if ( this == &other )
	{
		return *this;
	}

	m_id = other.m_id;

	return *this;
}


//-----------------------------------------------------------------------------------------------
HashedString& HashedString::operator=( const HashedString& other )
{
	m_id = other.m_id;

	return *this;
}


//-----------------------------------------------------------------------------------------------
bool HashedString::operator==( const HashedString& other )
{
	return m_id == other.m_id;
}


//-----------------------------------------------------------------------------------------------
bool HashedString::operator!=( const HashedString& other )
{
	return m_id != other.m_id;
}


//-----------------------------------------------------------------------------------------------
bool HashedString::AddToStringTable( const std::string& rawString )
{
	const auto& iter = s_stringTable.find( m_id );

	if ( iter == s_stringTable.cend() )
	{
		s_stringTable[m_id] = rawString;
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
const std::string HashedString::GetRawString() const
{
	const auto& iter = s_stringTable.find( m_id );

	if ( iter == s_stringTable.cend() )
	{
		return "";
	}

	return iter->second;
}

#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/IntVec2.hpp"


//-----------------------------------------------------------------------------------------------
void NamedStrings::PopulateFromXmlElementAttributes( const XmlElement& element )
{
	const XmlAttribute* attr = element.FirstAttribute();
	
	while ( attr != nullptr )
	{
		SetValue( attr->Name(), std::string( attr->Value() ) );
		attr = attr->Next();
	}
}


//-----------------------------------------------------------------------------------------------
void NamedStrings::SetValue( const std::string& keyName, const std::string& newValue )
{
	m_keyValuePairs[keyName] = newValue;
}


//-----------------------------------------------------------------------------------------------
bool NamedStrings::GetValue( const std::string& keyName, bool defaultValue ) const
{
	NamedStringsMap::const_iterator mapIter = m_keyValuePairs.find( keyName );
	if ( mapIter == m_keyValuePairs.cend() )
	{
		return defaultValue;
	}

	return atob( mapIter->second );
}


//-----------------------------------------------------------------------------------------------
int NamedStrings::GetValue( const std::string& keyName, int defaultValue ) const
{
	NamedStringsMap::const_iterator mapIter = m_keyValuePairs.find( keyName );
	if ( mapIter == m_keyValuePairs.cend() )
	{
		return defaultValue;
	}

	return atoi( mapIter->second.c_str() );
}


//-----------------------------------------------------------------------------------------------
float NamedStrings::GetValue( const std::string& keyName, float defaultValue ) const
{
	NamedStringsMap::const_iterator mapIter = m_keyValuePairs.find( keyName );
	if ( mapIter == m_keyValuePairs.cend() )
	{
		return defaultValue;
	}

	return (float)atof( mapIter->second.c_str() );
}


//-----------------------------------------------------------------------------------------------
std::string NamedStrings::GetValue( const std::string& keyName, std::string defaultValue ) const
{
	NamedStringsMap::const_iterator mapIter = m_keyValuePairs.find( keyName );
	if ( mapIter == m_keyValuePairs.cend() )
	{
		return defaultValue;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
std::string NamedStrings::GetValue( const std::string& keyName, const char* defaultValue ) const
{
	NamedStringsMap::const_iterator mapIter = m_keyValuePairs.find( keyName );
	if ( mapIter == m_keyValuePairs.cend() )
	{
		return defaultValue;
	}

	return mapIter->second.c_str();
}


//-----------------------------------------------------------------------------------------------
Rgba8 NamedStrings::GetValue( const std::string& keyName, const Rgba8& defaultValue ) const
{
	NamedStringsMap::const_iterator mapIter = m_keyValuePairs.find( keyName );
	if ( mapIter == m_keyValuePairs.cend() )
	{
		return defaultValue;
	}

	Rgba8 mapValue;
	mapValue.SetFromText( mapIter->second.c_str() );
	return mapValue;
}


//-----------------------------------------------------------------------------------------------
Vec2 NamedStrings::GetValue( const std::string& keyName, const Vec2& defaultValue ) const
{
	NamedStringsMap::const_iterator mapIter = m_keyValuePairs.find( keyName );
	if ( mapIter == m_keyValuePairs.cend() )
	{
		return defaultValue;
	}

	Vec2 mapValue;
	mapValue.SetFromText( mapIter->second.c_str() );
	return mapValue;
}


//-----------------------------------------------------------------------------------------------
IntVec2 NamedStrings::GetValue( const std::string& keyName, const IntVec2& defaultValue ) const
{
	NamedStringsMap::const_iterator mapIter = m_keyValuePairs.find( keyName );
	if ( mapIter == m_keyValuePairs.cend() )
	{
		return defaultValue;
	}

	IntVec2 mapValue;
	mapValue.SetFromText( mapIter->second.c_str() );
	return mapValue;
}


//-----------------------------------------------------------------------------------------------
Vec3 NamedStrings::GetValue( const std::string& keyName, const Vec3& defaultValue ) const
{
	NamedStringsMap::const_iterator mapIter = m_keyValuePairs.find( keyName );
	if ( mapIter == m_keyValuePairs.cend() )
	{
		return defaultValue;
	}

	Vec3 mapValue;
	mapValue.SetFromText( mapIter->second.c_str() );
	return mapValue;
}

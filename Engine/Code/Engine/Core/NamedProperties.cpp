#include "Engine/Core/NamedProperties.hpp"


//-----------------------------------------------------------------------------------------------
NamedProperties::~NamedProperties()
{
	std::map<std::string, TypedPropertyBase*>::iterator mapIter;

	for ( mapIter = m_keyValuePairs.begin(); mapIter != m_keyValuePairs.end(); mapIter++ )
	{
		delete( mapIter->second );
		mapIter->second = nullptr;
	}

	m_keyValuePairs.clear();
}


//-----------------------------------------------------------------------------------------------
void NamedProperties::PopulateFromXMLAttributes( const XmlElement& element )
{
	const XmlAttribute* attr = element.FirstAttribute();

	while ( attr != nullptr )
	{
		SetValue( attr->Name(), std::string( attr->Value() ) );
		attr = attr->Next();
	}
}


//-----------------------------------------------------------------------------------------------
//void NamedProperties::SetValue( const std::string& keyName, const std::string& value )
//{
//	m_keyValuePairs[keyName] = value;
//}
//
//
////-----------------------------------------------------------------------------------------------
//std::string NamedProperties::GetValue( const std::string& keyName, const std::string& defValue ) const
//{
//
//}

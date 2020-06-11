#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntRange.hpp"
#include "Engine/Math/FloatRange.hpp"


//-----------------------------------------------------------------------------------------------
std::string ParseXmlAttribute( const XmlElement& element, const char* attributeName, const std::string& defaultValue )
{
	const char* defaultValueCStr = defaultValue.c_str();

	element.QueryStringAttribute(attributeName, &defaultValueCStr );

	return std::string( defaultValueCStr );
}


//-----------------------------------------------------------------------------------------------
int ParseXmlAttribute( const XmlElement& element, const char* attributeName, int defaultValue )
{
	element.QueryIntAttribute( attributeName, &defaultValue );

	return defaultValue;
}


//-----------------------------------------------------------------------------------------------
char ParseXmlAttribute( const XmlElement& element, const char* attributeName, char defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	char value = defaultValue;
	if ( attributeValueText )
	{
		value = attributeValueText[0];
	}

	return value;
}


//-----------------------------------------------------------------------------------------------
bool ParseXmlAttribute( const XmlElement& element, const char* attributeName, bool defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	bool value = defaultValue;
	if ( attributeValueText )
	{
		value = atob( attributeValueText );
	}

	return value;
}


//-----------------------------------------------------------------------------------------------
float ParseXmlAttribute( const XmlElement& element, const char* attributeName, float defaultValue )
{
	element.QueryFloatAttribute( attributeName, &defaultValue );

	return defaultValue;
}


//-----------------------------------------------------------------------------------------------
Rgba8 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Rgba8& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Rgba8 value = defaultValue;
	if ( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//-----------------------------------------------------------------------------------------------
Vec2 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Vec2& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Vec2 value = defaultValue;
	if ( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//-----------------------------------------------------------------------------------------------
IntVec2 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const IntVec2& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	IntVec2 value = defaultValue;
	if ( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//-----------------------------------------------------------------------------------------------
AABB2 ParseXmlAttribute( const XmlElement& element, const char* attributeName, const AABB2& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	AABB2 value = defaultValue;
	if ( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//-----------------------------------------------------------------------------------------------
IntRange ParseXmlAttribute( const XmlElement& element, const char* attributeName, const IntRange& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	IntRange value = defaultValue;
	if ( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//-----------------------------------------------------------------------------------------------
FloatRange ParseXmlAttribute( const XmlElement& element, const char* attributeName, const FloatRange& defaultValue )
{
	const char* attributeValueText = element.Attribute( attributeName );
	FloatRange value = defaultValue;
	if ( attributeValueText )
	{
		value.SetFromText( attributeValueText );
	}
	return value;
}


//-----------------------------------------------------------------------------------------------
std::string ParseXmlAttribute( const XmlElement& element, const char* attributeName, const char* defaultValue )
{
	return ParseXmlAttribute( element, attributeName, std::string( defaultValue ) );
}


//-----------------------------------------------------------------------------------------------
Strings ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Strings& defaultValues )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Strings value = defaultValues;
	if ( attributeValueText )
	{
		value = SplitStringOnDelimiter( attributeValueText, ',' );
	}
	return value;
}


//-----------------------------------------------------------------------------------------------
Ints ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Ints& defaultValues )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Ints value = defaultValues;
	if ( attributeValueText )
	{
		Strings intStrings = SplitStringOnDelimiter( attributeValueText, ',' );
		value = ParseIntsFromStrings( intStrings );
	}
	return value;
}


//-----------------------------------------------------------------------------------------------
Floats ParseXmlAttribute( const XmlElement& element, const char* attributeName, const Floats& defaultValues )
{
	const char* attributeValueText = element.Attribute( attributeName );
	Floats value = defaultValues;
	if ( attributeValueText )
	{
		Strings floatStrings = SplitStringOnDelimiter( attributeValueText, ',' );
		value = ParseFloatsFromStrings( floatStrings );
	}
	return value;
}


//-----------------------------------------------------------------------------------------------
Ints ParseIntsFromStrings( const Strings& intStrings )
{
	Ints vectorOfInts;
	vectorOfInts.reserve( (int)intStrings.size() );

	for( int intStrIndex = 0; intStrIndex < (int)intStrings.size(); ++intStrIndex )
	{
		vectorOfInts.push_back( atoi( intStrings[intStrIndex].c_str() ) );
	}

	return vectorOfInts;
}


//-----------------------------------------------------------------------------------------------
Floats ParseFloatsFromStrings( const Strings& floatStrings )
{
	Floats vectorOfFloats;
	vectorOfFloats.reserve( (int)floatStrings.size() );

	for ( int floatStrIndex = 0; floatStrIndex < (int)floatStrings.size(); ++floatStrIndex )
	{
		vectorOfFloats.push_back( (float)atof( floatStrings[floatStrIndex].c_str() ) );
	}

	return vectorOfFloats;
}

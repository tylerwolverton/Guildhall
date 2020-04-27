#pragma once
#include "XmlUtils.hpp"

#include <string>
#include <map>

template <typename VALUE_TYPE>
class TypedProperty;

//-----------------------------------------------------------------------------------------------
class TypedPropertyBase
{
public:
	virtual ~TypedPropertyBase() {}

	virtual std::string GetAsString() const = 0;
	virtual void const* GetUniqueID() const = 0;

	template <typename T>
	bool Is() const
	{
		return GetUniqueID() == TypedProperty<T>::UNIQUE_ID;
	}

};


//-----------------------------------------------------------------------------------------------
template <typename VALUE_TYPE>
class TypedProperty : public TypedPropertyBase
{
public:
	virtual std::string GetAsString() const final { return ToString( m_value ); }
	virtual void const* GetUniqueID() const final { return UNIQUE_ID; }

public:
	// std::string m_key;
	VALUE_TYPE m_value;

public:
	static int s_whatever;
	static void const* const UNIQUE_ID = &s_whatever;
};


//-----------------------------------------------------------------------------------------------
class NamedProperties
{
public:
	~NamedProperties();

	//-----------------------------------------------------------------------------------------------
	void PopulateFromXMLAttributes( const XmlElement& element );

	//-----------------------------------------------------------------------------------------------
	//void SetValue( const std::string& keyName, const std::string& valueName );

	//-----------------------------------------------------------------------------------------------
	// base string version
	//std::string GetValue( const std::string& keyName, const std::string& defValue ) const;

	//-----------------------------------------------------------------------------------------------
	// for everything else, there's templates!
	template <typename T>
	void SetValue( std::string const& keyName, T const& value )
	{
		TypedPropertyBase* base = m_keyValuePairs.find( keyName );
		if ( base == nullptr )
		{
			// doesn't exist, make a new one            
			TypedProperty<T>* prop = new TypedProperty<T>();
			prop->m_value = value;
			m_keyValuePairs[keyName] = prop;

		}
		else
		{
			// already exists in map, update
			if ( base->Is<T>() )
			{
				TypedProperty<T>* prop = ( TypedProperty<T>* )base;
				prop->m_value = value;
			}
			else
			{
				// not the same thing
				// delete and remake
				delete base;
      
				TypedProperty<T>* prop = new TypedProperty<T>();
				prop->m_value = value;
				m_keyValuePairs[keyName] = prop;
			}
		}
	}


	//-----------------------------------------------------------------------------------------------
	template <typename T>
	T GetValue( std::string const& keyName, T const& defValue ) const
	{
		TypedPropertyBase* base = nullptr;
		auto iter = m_keyValuePairs.find( keyName );
		if ( iter != m_keyValuePairs.end() )
		{
			base = iter->second;

			// this works WITHOUT RTTI enabled
			// but will not work if prop is inherited from T
			if ( base->GetUniqueID() == TypedProperty<T>::UNIQUE_ID )
			{
				// make sure this is safe!  how....?
				TypedProperty<T>* prop = ( TypedProperty<T>* )base;
				return prop->m_value;
			}
			else
			{
				std::string strValue = base->GetAsString();
				return GetValueFromString( strValue, defValue );
			}
		}
		else
		{ // failed to find
			return defValue;
		}
	}

private:
	// this is going to be different
	// std::map<std::string, std::string> m_keyValuePairs;
	// std::map<std::string, void*> m_keyValuePointers;

	// We need to type type in a map
	// But we can't store the template argument... or can we?
	std::map<std::string, TypedPropertyBase*> m_keyValuePairs;
};

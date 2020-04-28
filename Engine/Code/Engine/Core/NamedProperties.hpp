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
		return GetUniqueID() == TypedProperty<T>::StaticUniqueId();
	}
};


//-----------------------------------------------------------------------------------------------
template <typename VALUE_TYPE>
class TypedProperty : public TypedPropertyBase
{
public:
	virtual std::string GetAsString() const final { return ToString( m_value ); }
	virtual void const* GetUniqueID() const final { return StaticUniqueId(); }

public:
	// std::string m_key;
	VALUE_TYPE m_value;

public:
	static void const* const StaticUniqueId()
	{
		static int s_local = 0;
		return &s_local;
	}

	template <typename VALUE_TYPE>
	std::string ToString( VALUE_TYPE value ) const
	{
		return "";
	}
};


//-----------------------------------------------------------------------------------------------
class NamedProperties
{
public:
	~NamedProperties();

	void PopulateFromXMLAttributes( const XmlElement& element );

	void SetValue( const std::string& keyname, const char* value );
	std::string GetValue( const std::string& keyName, const char* defaultValue ) const;

	//-----------------------------------------------------------------------------------------------
	// for everything else, there's templates!
	template <typename T>
	void SetValue( std::string const& keyName, T const& value )
	{
		TypedPropertyBase* base = FindInMap( keyName );
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
		TypedPropertyBase* base = FindInMap( keyName );
		if ( base == nullptr )
		{
			// this works WITHOUT RTTI enabled
			// but will not work if prop is inherited from T
			if ( base->Is<T>() )
			{
				// make sure this is safe!  how....?
				TypedProperty<T>* prop = ( TypedProperty<T>* )base;
				return prop->m_value;
			}
			else
			{
				std::string strValue = base->GetAsString();
				return GetValue( strValue, defValue );
			}
		}
		else
		{ // failed to find
			return defValue;
		}
	}

private:
	TypedPropertyBase* FindInMap( const std::string& key  ) const;

	// We need to type type in a map
	// But we can't store the template argument... or can we?
	std::map<std::string, TypedPropertyBase*> m_keyValuePairs;
};

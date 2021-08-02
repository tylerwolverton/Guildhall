#pragma once
#include <cstdint>
#include <map>
#include <string>


//-----------------------------------------------------------------------------------------------
class HashedString
{
public:
	HashedString() = default;
	HashedString( const char* input );
	HashedString( const std::string& input );

	HashedString( const HashedString& other );
	HashedString( const HashedString&& other );
	HashedString& operator=( const HashedString& other );
	HashedString& operator=( const HashedString&& other );
	
	bool operator==( const HashedString& other );
	bool operator!=( const HashedString& other );

	// TODO: Use const char* for this?
	const std::string GetRawString() const;

private:
	bool AddToStringTable( const std::string& rawString );

private:
	uint32_t m_id = 0U;

	static std::map<uint32_t, std::string> s_stringTable;
};

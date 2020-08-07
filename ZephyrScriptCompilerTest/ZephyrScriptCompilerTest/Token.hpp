#pragma once
#include "BytecodeCommon.hpp"


//-----------------------------------------------------------------------------------------------
class Token
{
public: 
	Token( eTokenType type, const char* start, int length, int lineNum );

	eTokenType GetType() const { return m_type; }
	const char* GetStart() const { return m_start; }
	int GetLength() const { return m_length; }
	int GetLineNum() const { return m_lineNum; }

private:
	eTokenType m_type = eTokenType::UNKNOWN;
	const char* m_start = nullptr;
	int m_length = 0;
	int m_lineNum = 0;
};

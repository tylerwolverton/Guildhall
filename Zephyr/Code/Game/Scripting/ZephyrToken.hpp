#pragma once
#include "Game/Scripting/ZephyrCommon.hpp"

#include <string>

//-----------------------------------------------------------------------------------------------
class ZephyrToken
{
public:
	ZephyrToken( eTokenType type, int lineNum );

	eTokenType	GetType() const							{ return m_type; }
	int			GetLineNum() const						{ return m_lineNum; }

	std::string GetDebugName() const;

private:
	eTokenType	m_type = eTokenType::UNKNOWN;
	int			m_lineNum = 0;
};

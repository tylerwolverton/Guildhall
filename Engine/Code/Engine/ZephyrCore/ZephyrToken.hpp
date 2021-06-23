#pragma once
#include "Engine/ZephyrCore/ZephyrCommon.hpp"

#include <string>

//-----------------------------------------------------------------------------------------------
class ZephyrToken
{
public:
	ZephyrToken() {}
	ZephyrToken( eTokenType type, const std::string& data, int lineNum );

	eTokenType	GetType() const							{ return m_type; }
	std::string	GetData() const							{ return m_data; }
	int			GetLineNum() const						{ return m_lineNum; }

	std::string GetDebugName() const;

private:
	eTokenType	m_type = eTokenType::UNKNOWN;
	std::string m_data;
	int			m_lineNum = 0;
};

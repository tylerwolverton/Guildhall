#pragma once
#include "Engine/Math/Vec2.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class BlockDefinition
{
public:
	BlockDefinition( const std::string& name );

private:
	// TEMP
	void AddToVector( const BlockDefinition& blockDef );

private:
	std::string m_name;

	bool m_isVisible = true;
	bool m_isSolid = false;
	bool m_isOpaque = true;

	Vec2 m_uvTop;
	Vec2 m_uvSide;
	Vec2 m_uvBottom;

	static std::vector<BlockDefinition> s_blockDefs;
};

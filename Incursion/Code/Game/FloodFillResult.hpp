#pragma once
#include <vector>


//-----------------------------------------------------------------------------------------------
struct FloodFillResult
{
public:
	bool				m_isExitReachable = false;
	std::vector<bool>	m_tilesProcessed;
	std::vector<bool>	m_tilesReachable;
};

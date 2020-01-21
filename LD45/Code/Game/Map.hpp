#pragma once
#include "Game/Tile.hpp"

#include <vector>

class Map
{
public:
	Map( int width, int height );
	~Map();

	void Render() const;

private:
	int m_width;
	int m_height;
	std::vector<Tile> m_tiles;
};
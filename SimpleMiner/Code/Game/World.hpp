#pragma once
#include "Game/Chunk.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class World
{
public:
	World();
	~World();

	void Update( float deltaSeconds );
	void Render() const;
	void DebugRender() const;

private:
	std::vector<Chunk> m_chunks;
};

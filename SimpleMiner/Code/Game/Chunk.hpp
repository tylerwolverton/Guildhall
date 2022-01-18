#pragma once
#include "Game/Block.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/CPUMesh.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/GPUMesh.hpp"


//-----------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------------
class Chunk
{
public:
	void Render();


private:
	Block m_blocks[NUM_BLOCKS_IN_CHUNK];

	IntVec2 m_worldCoords;
	AABB3 m_worldBounds;

	CPUMesh m_cpuMesh;
	GPUMesh m_gpuMesh;
};

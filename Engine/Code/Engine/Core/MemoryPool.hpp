#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
struct ObjectHeader
{
public:
	byte* address = nullptr;
	byte* next = nullptr;
	byte* prev = nullptr;
	int dataSize = -1;
	bool isFree = true; // needed?
};

constexpr int HEADER_SIZE = sizeof( ObjectHeader );


//-----------------------------------------------------------------------------------------------
class MemoryPool
{
public:
	MemoryPool( int sizeInBytes );
	virtual ~MemoryPool();

	byte* AllocateObject( int sizeInBytes, int alignment );
	byte* AllocateObjects( int sizeInBytes, int alignment, int numObjects );
	void Deallocate( byte* objectAddr );


private:
	byte* m_headObj = nullptr;
};

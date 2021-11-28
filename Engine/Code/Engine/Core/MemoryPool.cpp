#include "Engine/Core/MemoryPool.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

#include <memory>


//-----------------------------------------------------------------------------------------------
MemoryPool::MemoryPool( int sizeInBytes )
{
	GUARANTEE_OR_DIE( sizeInBytes > HEADER_SIZE, "Cannot create a memory pool smaller than header size of an object" );

	m_headObj = (byte*)malloc( sizeInBytes );

	ObjectHeader* header = reinterpret_cast<ObjectHeader*>( m_headObj );
	if ( header == nullptr )
	{
		return;
	}

	header->address = m_headObj;
	header->prev = nullptr;
	header->next = nullptr;
	header->dataSize = sizeInBytes - HEADER_SIZE;
	header->isFree = true;
}


//-----------------------------------------------------------------------------------------------
MemoryPool::~MemoryPool()
{
	free( m_headObj );
	m_headObj = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Allocates on the end of first node with enough space
// Should this look for smallest block big enough? How does that affect cache coherency?
//-----------------------------------------------------------------------------------------------
byte* MemoryPool::AllocateObject( int sizeInBytes, int alignment )
{
	GUARANTEE_OR_DIE( alignment != 0, "Can't allocate memory block with alignment 0" );

	int numOfAlignmentByteSections = sizeInBytes / alignment;
	int alignedSizeInBytes = numOfAlignmentByteSections * alignment;
	// Account for padding
	if ( alignedSizeInBytes < sizeInBytes )
	{
		alignedSizeInBytes += alignment;
	}

	byte* curBlock = m_headObj;

	while ( curBlock != nullptr )
	{
		ObjectHeader* curHeader = reinterpret_cast<ObjectHeader*>( curBlock );
		if ( !curHeader->isFree 
			 || curHeader->dataSize < alignedSizeInBytes )
		{
			curBlock = curHeader->next;
			continue;
		}

		// TODO: Account for block having enough space for data but not another header		
		byte* nextBlock = curBlock + alignedSizeInBytes + HEADER_SIZE;
		ObjectHeader* nextHeader = reinterpret_cast<ObjectHeader*>( nextBlock );

		// Create new header
		nextHeader->address = nextBlock;
		nextHeader->prev = curBlock;
		nextHeader->next = curHeader->next;
		nextHeader->dataSize = curHeader->dataSize - alignedSizeInBytes - HEADER_SIZE;
		nextHeader->isFree = true;

		// Update this header
		curHeader->next = nextBlock;
		curHeader->dataSize = alignedSizeInBytes;
		curHeader->isFree = false;

		return curBlock;
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void MemoryPool::Deallocate( byte* objectAddr )
{

}

#pragma once
#include "Engine/Renderer/RenderBuffer.hpp"


//-----------------------------------------------------------------------------------------------
struct BufferAttribute;


//-----------------------------------------------------------------------------------------------
class IndexBuffer : public RenderBuffer
{
public:
	IndexBuffer( RenderContext* owner, eRenderMemoryHint memHint );

	void Update( uint indexCount, const uint* indices );
	void Update( const std::vector<uint>& indices ); // helper, calls one above

};
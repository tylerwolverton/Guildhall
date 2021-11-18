#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include "Engine/Renderer/RenderCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
struct BufferAttribute
{
public:
	std::string m_name; 											// used to link to a D3D11 shader
	// uint location; 												// used to link to a GL/Vulkan shader 
	eBufferFormatType m_bufferFormatType = BUFFER_FORMAT_UNKNOWN;	// what data are we describing
	uint m_offset = 0; 												// where is it relative to the start of a vertex

public:
	BufferAttribute( const char* name, eBufferFormatType bufferFormatType, uint offset );
	BufferAttribute() = default;
};

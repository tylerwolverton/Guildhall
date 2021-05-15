#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>

// TODO: Move to D3D11Common
//-----------------------------------------------------------------------------------------------
enum eBufferFormatType : uint
{
	// add types as you need them - for now, we can get by with just this
	BUFFER_FORMAT_UNKNOWN,
	BUFFER_FORMAT_VEC2,
	BUFFER_FORMAT_VEC3,
	BUFFER_FORMAT_VEC4,
	BUFFER_FORMAT_R8G8B8A8_UNORM,
	BUFFER_FORMAT_INT
};


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

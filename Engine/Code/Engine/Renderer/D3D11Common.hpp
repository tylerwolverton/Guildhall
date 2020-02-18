#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
// Forward declarations
enum eBufferFormatType : uint;

//-----------------------------------------------------------------------------------------------
#if !defined(WIN32_LEAN_AND_MEAN) 
#define WIN32_LEAN_AND_MEAN
#endif


//-----------------------------------------------------------------------------------------------
// DX3D11 Includes
#define INITGUID
#include <d3d11.h>  // d3d11 specific objects
#include <dxgi.h>   // shared library used across multiple dx graphical interfaces
#include <dxgidebug.h>  // debug utility (mostly used for reporting and analytics)


//-----------------------------------------------------------------------------------------------
#define RENDER_DEBUG
#define DX_SAFE_RELEASE(obj)  if (nullptr != (obj)) { (obj)->Release(); (obj) = nullptr; }


//-----------------------------------------------------------------------------------------------
// The "WHAT" are we using it for
enum eRenderBufferUsageBit : uint
{
	VERTEX_BUFFER_BIT = BIT_FLAG( 0 ),	// A02: can be used to store vertices
	INDEX_BUFFER_BIT = BIT_FLAG( 1 ),  	// we will discuss this later
	UNIFORM_BUFFER_BIT = BIT_FLAG( 2 ),	// A03: used to store constants
};
typedef uint eRenderBufferUsage;			// typedef helps to describe to reader when an entire bit field can be sent as opposed to a single bit


//-----------------------------------------------------------------------------------------------
// The "HOW" are we going to access it
enum eRenderMemoryHint : uint
{
	MEMORY_HINT_GPU,		// GPU can read/write, CPU can't touch it. If we change it, it changes rarely from CPU
	MEMORY_HINT_DYNAMIC, 	// GPU memory (read/write), that changes OFTEN from CPU - it allows "Mapping" of memory 
	MEMORY_HINT_STAGING,	// CPU memory (read/write), can copy to a GPU buffer
};

D3D11_USAGE ToDxMemoryUsage( eRenderMemoryHint hint );
UINT ToDXUsage( eRenderBufferUsage usage );
DXGI_FORMAT ToDXGIFormat( eBufferFormatType formatType );


//-----------------------------------------------------------------------------------------------
static const char* DEFAULT_SHADER_FILENAME = "Data/Shaders/Default.hlsl";
static const char* ERROR_SHADER_FILENAME = "Data/Shaders/Error.hlsl";

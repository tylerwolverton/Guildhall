#pragma once

//TODO Rename to D3D11Common

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
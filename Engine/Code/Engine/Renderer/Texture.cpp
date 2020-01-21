#include "Engine/Renderer/Texture.hpp"


//-----------------------------------------------------------------------------------------------
Texture::Texture( int id, const char* filePath, const IntVec2& texelSize )
	: m_id( id )
	, m_filePath( filePath )
	, m_texelSize( texelSize )
{
}

//-----------------------------------------------------------------------------------------------
Texture::~Texture()
{
}

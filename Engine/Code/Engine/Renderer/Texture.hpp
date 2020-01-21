#pragma once
#include "Engine/Math/IntVec2.hpp"


//-----------------------------------------------------------------------------------------------
class Texture
{
public:
	Texture( int id, const char* filePath, const IntVec2& texelSize );
	~Texture();
	
	unsigned int GetTextureID()	const				{ return m_id; }
	const char*  GetFilePath()	const				{ return m_filePath; }
	IntVec2		 GetTexelSize()	const				{ return m_texelSize; }

private:
	int			m_id = -1;
	const char* m_filePath = nullptr;
	IntVec2		m_texelSize = IntVec2::ZERO;
};
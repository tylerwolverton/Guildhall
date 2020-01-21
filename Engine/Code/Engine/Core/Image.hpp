#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/IntVec2.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Image
{
public:
	Image( const char* imageFilePath );
	~Image();
	
	const std::string&	GetImageFilePath() const									{ return m_imageFilePath; }
	IntVec2				GetDimensions( int numRotations = 0 ) const;
	Rgba8				GetTexelColor( int requestedTexelX, int requestedTexelY, int numRotations = 0, bool isMirrored = false ) const;
	Rgba8				GetTexelColor( const IntVec2& texelCoords, int numRotations = 0, bool isMirrored = false ) const;
	void				SetTexelColor( int texelX, int texelY, const Rgba8 newColor );
	void				SetTexelColor( const IntVec2& texelCoords, const Rgba8 newColor );

private:
	int GetClampedNum90DegreeRotations( int numRotations ) const;

private:
	std::string			m_imageFilePath;
	IntVec2				m_dimensions = IntVec2( 0, 0 );
	std::vector<Rgba8>	m_rgbaTexels;
};

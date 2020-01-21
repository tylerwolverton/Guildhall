#include "Engine/Core/Image.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"


//-----------------------------------------------------------------------------------------------
Image::Image( const char* imageFilePath )
	: m_imageFilePath( std::string( imageFilePath ) )
{
	int imageTexelSizeX = 0; // This will be filled in for us to indicate image width
	int imageTexelSizeY = 0; // This will be filled in for us to indicate image height
	int numComponents = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

	// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load( 1 ); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* imageData = stbi_load( imageFilePath, &imageTexelSizeX, &imageTexelSizeY, &numComponents, numComponentsRequested );

	// Check if the load was successful
	GUARANTEE_OR_DIE( imageData, Stringf( "Failed to load image \"%s\"", imageFilePath ) );
	GUARANTEE_OR_DIE( numComponents >= 3 && numComponents <= 4 && imageTexelSizeX > 0 && imageTexelSizeY > 0, Stringf( "ERROR loading image \"%s\" (Bpp=%i, size=%i,%i)", imageFilePath, numComponents, imageTexelSizeX, imageTexelSizeY ) );

	// Set member variables from image data
	m_dimensions = IntVec2( imageTexelSizeX, imageTexelSizeY );

	// Reserve a spot for each texel in the image
	int texelIndex = 0;
	int numTexels = imageTexelSizeX * imageTexelSizeY;
	m_rgbaTexels.reserve( numTexels );

	// Load data in rgb(a) chunks from raw image data
	int numImageBytes = numTexels * numComponents;
	for ( int imageDataIndex = 0; imageDataIndex < numImageBytes; imageDataIndex += numComponents )
	{
		unsigned char r = imageData[ imageDataIndex ];
		unsigned char g = imageData[ imageDataIndex + 1 ];
		unsigned char b = imageData[ imageDataIndex + 2 ];

		unsigned char a = 255;
		if ( numComponents == 4 )
		{
			a = imageData[ imageDataIndex + 3 ];
		}

		m_rgbaTexels.push_back( Rgba8( r, g, b, a ) );
		++texelIndex;
	}

	stbi_image_free( imageData );
}


//-----------------------------------------------------------------------------------------------
Image::~Image()
{
}


//-----------------------------------------------------------------------------------------------
IntVec2 Image::GetDimensions( int numRotations ) const
{
	if ( numRotations % 2 == 0 )
	{
		return m_dimensions;
	}
	else
	{
		return IntVec2( m_dimensions.y, m_dimensions.x );
	}
}


//-----------------------------------------------------------------------------------------------
Rgba8 Image::GetTexelColor( int requestedTexelX, int requestedTexelY, int numRotations, bool isMirrored ) const
{
	GUARANTEE_OR_DIE( requestedTexelX >= 0 && requestedTexelY >= 0, Stringf( "Negative index for texel data requested for image \"%s\"", m_imageFilePath.c_str() ) );

	int numRotationsInRange = GetClampedNum90DegreeRotations( numRotations );
	
	GUARANTEE_OR_DIE( requestedTexelX < GetDimensions( numRotationsInRange ).x  && requestedTexelY < GetDimensions( numRotationsInRange ).y, Stringf( "Out of bounds index ( %d, %d ) requested for image \"%s\"", requestedTexelX, requestedTexelY, m_imageFilePath.c_str() ) );
	
	int imageTexelX = requestedTexelX;
	int imageTexelY = requestedTexelY;
	switch ( numRotationsInRange )
	{
		// 0 degrees
		case 0:
		{

			break;
		}
		// 90 degrees
		case 1:
		{
			imageTexelX = requestedTexelY;
			imageTexelY = m_dimensions.y - requestedTexelX - 1;
			break;
		}
		// 180 degrees
		case 2:
		{
			imageTexelX = m_dimensions.x - requestedTexelX - 1;
			imageTexelY = m_dimensions.y - requestedTexelY - 1;
			break;
		}
		// 270 degrees
		case 3:
		{
			imageTexelX = requestedTexelY;
			imageTexelY = m_dimensions.y - requestedTexelX - 1;

			imageTexelX = m_dimensions.x - imageTexelX - 1;
			imageTexelY = m_dimensions.y - imageTexelY - 1;
			break;
		}
	}

	if ( isMirrored )
	{
		imageTexelX = m_dimensions.x - imageTexelX - 1;
	}

	int texelIndex = imageTexelX + ( imageTexelY * m_dimensions.x );
	
	GUARANTEE_OR_DIE( texelIndex < (int)m_rgbaTexels.size(), Stringf( "Texel index out of bounds for image \"%s\"", m_imageFilePath.c_str() ) );

	return m_rgbaTexels[texelIndex];
}


//-----------------------------------------------------------------------------------------------
Rgba8 Image::GetTexelColor( const IntVec2& texelCoords, int numRotations, bool isMirrored ) const
{
	return GetTexelColor( texelCoords.x, texelCoords.y, numRotations, isMirrored );
}


//-----------------------------------------------------------------------------------------------
void Image::SetTexelColor( int texelX, int texelY, const Rgba8 newColor )
{
	GUARANTEE_OR_DIE( texelX >= 0 && texelY >= 0, Stringf( "Negative index for texel data requested for image \"%s\"", m_imageFilePath.c_str() ) );

	int texelIndex = texelX + ( texelY * m_dimensions.x );

	GUARANTEE_OR_DIE( texelIndex < (int)m_rgbaTexels.size(), Stringf( "Texel index out of bounds for image \"%s\"", m_imageFilePath.c_str() ) );

	m_rgbaTexels[texelIndex] = newColor;
}


//-----------------------------------------------------------------------------------------------
void Image::SetTexelColor( const IntVec2& texelCoords, const Rgba8 newColor )
{
	SetTexelColor( texelCoords.x, texelCoords.y, newColor );
}


//-----------------------------------------------------------------------------------------------
int Image::GetClampedNum90DegreeRotations( int numRotations ) const
{
	// Adjust num rotations to be between -3 and 3, increments of 90 degrees only
	int numRotationsInRange = numRotations;
	if ( numRotationsInRange >= 0 )
	{
		numRotationsInRange %= 4;
	}
	else
	{
		// Bring within -1 to -4
		while ( numRotationsInRange < -4 )
		{
			numRotationsInRange += 4;
		}

		// Number of negative rotations is equal to 4 (max rotations) - num negative rotations, so adding the 
		// existing negative rotation count to 4 will leave us with the number of positive rotations to perform
		numRotationsInRange = numRotationsInRange + 4;
	}

	GUARANTEE_OR_DIE( numRotationsInRange >= 0 && numRotationsInRange < 4, Stringf( "Tried to rotate image %d times, must be within 0 to 3!", numRotationsInRange ) );

	return numRotationsInRange;
}

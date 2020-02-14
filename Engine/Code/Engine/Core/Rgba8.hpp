#pragma once


//-----------------------------------------------------------------------------------------------
struct Rgba8
{
public:
	unsigned char r = 255;
	unsigned char g = 255;
	unsigned char b = 255;
	unsigned char a = 255;

	//-----------------------------------------------------------------------------------------------
	// Common colors
	static const Rgba8 WHITE;
	static const Rgba8 BLACK;
	static const Rgba8 RED;
	static const Rgba8 DARK_RED;
	static const Rgba8 GREEN;
	static const Rgba8 DARK_GREEN;
	static const Rgba8 BLUE;
	static const Rgba8 DARK_BLUE;
	static const Rgba8 MAGENTA;
	static const Rgba8 DARK_MAGENTA;
	static const Rgba8 CYAN;
	static const Rgba8 DARK_CYAN;
	static const Rgba8 YELLOW;
	static const Rgba8 DARK_YELLOW;
	static const Rgba8 ORANGE;
	static const Rgba8 PURPLE;
	static const Rgba8 GREY;
	static const Rgba8 DARK_GREY;

public:
	// Construction
	Rgba8() {}  // Do nothing
	explicit Rgba8( unsigned char initialR, unsigned char initialG, unsigned char initialB, unsigned char initialA = 255 );
	void SetFromText( const char* asText );

	bool IsRGBEqual( const Rgba8& otherColor );

	bool		operator==( const Rgba8& compare ) const;		// Rgba8 == Rgba8
	bool		operator!=( const Rgba8& compare ) const;		// Rgba8 != Rgba8
};

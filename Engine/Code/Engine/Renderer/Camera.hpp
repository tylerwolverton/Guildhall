#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
class Camera
{
public:
	// can use this to determine aspect ratio
	void SetOutputSize( const Vec2& size );

	// where is the camera.  With orthographic 
	// and no ration, this is effectively where you are looking
	// - ie, the screen will be centered here.
	// - note: ignore z for now
	void SetPosition( const Vec3& position );

	// sets orthographic so that your screen is "height" high.  
	// can determine min and max from height, aspect ratio, and position
	void SetProjectionOrthographic( float height, float nearZ = -1.f, float farZ = 1.f );

	// If you use the above functions, you have all the information you need for this
	// and you're a little more future proof for SD2
	Vec2 ClientToWorldPosition( const Vec2& clientPos );
	
	// helpers
	float GetAspectRatio() const;

	void SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight );
	void SetOrthoView( const AABB2& cameraBounds );
	//void Translate2D( const Vec2& translation);

	Vec2 GetOrthoBottomLeft()			const	{ return m_bottomLeft; }
	Vec2 GetOrthoTopRight()				const	{ return m_topRight; }
	//const AABB2 GetOrthoBounds()		const	{ return AABB2( m_bottomLeft, m_topRight ); }

//private:
	// used internally for now - these will eventually go away
	// but for now you can compute these from the values set above
	// to use in ClientToWorldPosition
	Vec2 GetOrthoMin() const;
	Vec2 GetOrthoMax() const;

private:
	Vec2 m_bottomLeft;
	Vec2 m_topRight;
	Vec2 m_outputSize;
	Vec3 m_position;

};

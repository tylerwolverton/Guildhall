#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
class Camera
{
public:
	void SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight );
	void SetOrthoView( const AABB2& cameraBounds );
	void Translate2D( const Vec2& translation);

	Vec2 GetOrthoBottomLeft()			const	{ return m_bottomLeft; }
	Vec2 GetOrthoTopRight()				const	{ return m_topRight; }
	//const AABB2 GetOrthoBounds()		const	{ return AABB2( m_bottomLeft, m_topRight ); }

private:
	Vec2 m_bottomLeft;
	Vec2 m_topRight;
};


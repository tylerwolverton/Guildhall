#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"

class Renderer;

class UILabel
{
public:

	virtual void Render( Renderer* renderer ) = 0;

protected:
	AABB2 m_boundingBox;
	Rgba8 m_tint = Rgba8::WHITE;
};

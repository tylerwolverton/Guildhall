#pragma once
#include "Engine/Core/Rgba8.hpp"


//-----------------------------------------------------------------------------------------------
class Rigidbody2D;


//-----------------------------------------------------------------------------------------------
class GameObject
{
public:
	/*GameObject();*/
	~GameObject();

public:
	Rigidbody2D* m_rigidbody = nullptr;
	Rgba8 m_borderColor = Rgba8::MAGENTA;
	Rgba8 m_fillColor = Rgba8::MAGENTA;
};

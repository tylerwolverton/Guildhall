#pragma once

class Rigidbody2D;


class GameObject
{
public:
	/*GameObject();*/
	~GameObject();

public:
	Rigidbody2D* m_rigidbody;
};

#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class World
{
public:
	World();
	~World();

	void Update( float deltaSeconds );
	void Render() const;
	void DebugRender() const;
};

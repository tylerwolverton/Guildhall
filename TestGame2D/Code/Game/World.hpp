#pragma once
#include <vector>


//-----------------------------------------------------------------------------------------------
class Map;


//-----------------------------------------------------------------------------------------------
class World
{
public:
	World();
	~World();

	void Update( float deltaSeconds );
	void Render() const;
	void DebugRender() const;

	void BuildNewMap( int width, int height );

private:
	Map* m_curMap = nullptr;
};
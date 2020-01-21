#pragma once
#include <string>


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

	void BuildNewMap( std::string name );

private:
	Map* m_curMap = nullptr;
};
#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class Clock;
class Map;

//-----------------------------------------------------------------------------------------------
class World
{
public:
	World( Clock* gameClock );
	~World();

	void Update();
	void Render() const;
	void DebugRender() const;

	void BuildNewMap( std::string name );

private:
	Map* m_curMap = nullptr;
	Clock* m_worldClock = nullptr;
};
#pragma once
#include <vector>

class Game;
class Map;

class World
{
public:
	World( Game* game);
	~World();

	void Render() const;

private:
	Game* m_game = nullptr;
	Map* m_curMap = nullptr;
};
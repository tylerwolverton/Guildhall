#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <vector>

//-----------------------------------------------------------------------------------------------
class Entity;
class Player;
struct Rgba8;
class RandomNumberGenerator;
class Camera;
class World;


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void Startup();
	void Update( float deltaSeconds );
	void Render() const;
	void DebugRender() const;
	void Shutdown();

	void CheckCollisions();
	void RestartGame();

	void AddVertexesToPlayer( std::vector<Vertex_PCU> vertexes, const Rgba8& color );
	void SpawnEntity( Entity* entity, const Vec2& position, const Vec2& initialVelocity );
	void SpawnEnemy( std::vector<Vertex_PCU> vertexes, const Rgba8& color, const Vec2& position, const Vec2& initialVelocity );

	Entity* GetPlayer()											{ return (Entity*)m_player; }

	std::vector<Vertex_PCU> GetPrettyTriangle();
	void AddScreenShakeIntensity(float additionalIntensityFraction);

	void SpawnDebris( const Vec2& position, const Vec2& velocity, const Rgba8& color, int numPieces );

	void SetDebugRendering( bool isDebugRendering ) { m_isDebugRendering = isDebugRendering; }
	bool GetDebugRendering() { return m_isDebugRendering; }

public:
	RandomNumberGenerator* m_randNumGen = nullptr;

private:
	void UpdateCameras( float deltaSeconds );
	
	void UpdateEntities( float deltaSeconds );
	void RenderEntities() const;
	void DebugRenderEntities() const;
	void DeleteGarbageEntities();

	void SpawnNewEntities( float deltaSeconds );
	void MakePrettyTriangles();

private:
	std::vector<Entity*> m_entities;
	Player* m_player = nullptr;

	World* m_world = nullptr;

	bool m_isDebugRendering = false;

	float m_screenShakeIntensity = 0.f;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;

	std::vector<std::vector<Vertex_PCU>> m_triangleVertexGroups;

	float m_triangleSpawnTimer = 0.f;
	float m_boxSpawnTimer = 0.f;
	float m_boxSizeModifier = 1.f;
};

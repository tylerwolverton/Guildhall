#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Game/Entity.hpp"
#include "GameCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Texture;
class BitmapFont;


//-----------------------------------------------------------------------------------------------
class Player :
	public Entity
{
	friend class Map;

public:
	Player( Map* map, EntityFaction faction, const Vec2& position );
	~Player();

	virtual void Update( float deltaSeconds );
	virtual void Render() const;
	virtual void RenderHUD() const;
	virtual void DebugRender() const;
	virtual void Die();
	virtual void TakeDamage( int damage );
	
	void SetMap(Map* map)								{ m_map = map; }

	void UpdateInput( float deltaSeconds );

private:
	void PopulateVertexes();

	void UpdateFromKeyboard( float deltaSeconds );
	void UpdateFromGamepad( float deltaSeconds );

	void FireBullet();
	void Respawn();

	float GetPlayerGunWorldOrientation() const;

private:
	// Game state
	int		m_controllerID = 0;
	float	m_curDeathSeconds = 0.f;
	float	m_curDeathDismissalSeconds = 0.f;
	int		m_numDeaths = 0;

	// Physics
	Vec2	m_startingPosition = Vec2::ZERO;
	float	m_gunOrientationDegrees = 0.f;

	// Visual
	std::vector<Vertex_PCU> m_vertexes;
	Texture*				m_tankBodyTexture = nullptr;
	Texture*				m_tankGunTexture = nullptr;
	BitmapFont*				m_font = nullptr;

	// Audio
	SoundID m_diedSound;
	SoundID m_hitSound;
	SoundID m_shootSound;
};

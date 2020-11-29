#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
enum eEntityType : int;
struct AABB2;
class Clock;
class Entity;
class KeyButtonState;
class RandomNumberGenerator;
class Camera;
class GPUMesh;
class Material;
class TextBox;
class Texture;
class UIPanel;
class UISystem;
class World;


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	virtual void	Startup();
	virtual void	Update();
	virtual void	UpdateWorldMesh();
	virtual void	UpdateEntityAnimations();
	virtual void	Render() const;
	virtual void	Shutdown();

	virtual void	RestartGame();
	
	const World*	GetWorld()																{ return m_world; }
		
	void			WarpToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );

	Entity* CreateEntityInCurrentMap( eEntityType entityType, const Vec2& position, float yawOrientationDegrees );
	Entity* CreateEntityInCurrentMap( EntityId id, eEntityType entityType, const Vec2& position, float yawOrientationDegrees );
	Entity* GetEntityById( EntityId id );
	std::vector<Entity*> GetEntitiesInCurrentMap();
	std::vector<Entity*> GetLivingEntitiesInCurrentMap();
	void DeleteAllEntities();

	void MovePlayer( Entity* player, const Vec2& translationVec );
	void MoveEntity( EntityId entityId, const Vec2& translationVec );
	virtual void ShootEntity( EntityId shooterId, const Vec3& forwardVector, float shotRange, int damage );
	void RotateEntity( EntityId entityId, float yawRotationDegrees );
	void SetEntityPosition( EntityId entityId, const Vec2& newPosition );
	void SetEntityOrientation( EntityId entityId, float yawOrientationDegrees );
	void PossessEntity( Entity*& player, const Transform& cameraTransform );
	void UnpossessEntity( Entity*& player );

	float GetLastDeltaSeconds() const;
	void SetServerLastDeltaSeconds( float deltaSeconds )									{ m_serverLastDeltaSeconds = deltaSeconds; }
	
	virtual void AddPlayerScore( int playerNum, EntityId playerId );
	virtual void UpdatePlayerScore( int playerNum, int newScore );
	virtual std::vector<int> GetPlayerScores() const;

public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void LoadAssets();
	void LoadXmlEntityTypes();
	void LoadXmlMapMaterials();
	void LoadXmlMapRegions();
	void LoadXmlMaps();
	void ChangeMap( const std::string& mapName );
		
	// Events
	void WarpMapCommand( EventArgs* args );

protected:
	bool m_isGameStarted = false;

	Clock* m_gameClock = nullptr;
	float m_serverLastDeltaSeconds = -1.f;

	World* m_world = nullptr;
	std::string m_curMapStr;
			
	// Sounds
	SoundID m_testSound;
	
	// Default map data
	std::string m_defaultMaterialStr;
	std::string m_defaultMapRegionStr;
};

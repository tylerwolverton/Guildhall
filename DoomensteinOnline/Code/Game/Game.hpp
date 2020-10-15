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
	virtual void	Render() const;
	virtual void	Shutdown();

	virtual void	RestartGame();
	
	const World*	GetWorld()																{ return m_world; }
		
	void			WarpToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );

	void MovePlayer( Entity* player, const Vec2& translationVec );
	void SetPlayerOrientation( Entity* player, float yawOrientationDegrees );
	void PossessEntity( Entity*& player, const Transform& cameraTransform );
	void UnpossessEntity( Entity*& player );
	
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

private:
	Clock* m_gameClock = nullptr;

	World* m_world = nullptr;
	std::string m_curMapStr;
			
	// Sounds
	SoundID m_testSound;
	
	// Default map data
	std::string m_defaultMaterialStr;
	std::string m_defaultMapRegionStr;
};

#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class Camera;
class Entity;
class Material;
class SpriteAnimDefinition;
class SpriteSheet;
class RenderContext;
class Texture;
class Transform;


//-----------------------------------------------------------------------------------------------
struct DirectionAnimation
{
public:
	Vec2 facingDirection = Vec2::ZERO;
	SpriteAnimDefinition* animDef = nullptr;

public: 
	~DirectionAnimation() { PTR_SAFE_DELETE( animDef ); }
};


//-----------------------------------------------------------------------------------------------
class SpriteAnimationSetDefinition
{
public:
	SpriteAnimationSetDefinition( SpriteSheet* spriteSheet, Material* spriteMaterial, const XmlElement& spriteAnimSetDefElem );
	~SpriteAnimationSetDefinition();

	SpriteAnimDefinition*	GetSpriteAnimationDefForDirection( const Vec2& entityPos, float entityOrientationDegrees, const Camera& camera );

	void					AddFrameEvent( int frameNum, const std::string& eventName );
	void					FireFrameEvent( int frameNum, Entity* parent );

	void					AdjustAnimationSpeed( float deltaSpeedModifier );

	int						GetNumFrames() const; // Assumes directional anims all have same number of frames
	const Texture&			GetTexture() const;
	Material*				GetMaterial() const													{ return m_spriteMaterial; }

private:
	void AddDirectionAnimation( const std::string& animName, const Vec2& facingDir, const XmlElement& spriteAnimSetDefElem );

private:
	std::map< std::string, DirectionAnimation* > m_directionSpriteAnims;
	std::map<int, std::string> m_frameToEventNames;

	std::string		m_name;
	float			m_curSpeedModifier = 1.f;
	Material*		m_spriteMaterial = nullptr;
	SpriteSheet*	m_spriteSheet = nullptr;
};

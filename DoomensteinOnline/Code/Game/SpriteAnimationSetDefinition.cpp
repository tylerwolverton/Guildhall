#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
SpriteAnimationSetDefinition::SpriteAnimationSetDefinition( SpriteSheet* spriteSheet, const XmlElement& spriteAnimSetDefElem )
	: m_spriteSheet( spriteSheet )
{
	m_name = spriteAnimSetDefElem.Name();

	AddDirectionAnimation( "front",			Vec2( 1.f, 0.f ),						spriteAnimSetDefElem );
	AddDirectionAnimation( "frontLeft",		Vec2( SQRT_2_OVER_2, SQRT_2_OVER_2 ),	spriteAnimSetDefElem );
	AddDirectionAnimation( "left",			Vec2( 0.f, 1.f ),						spriteAnimSetDefElem );
	AddDirectionAnimation( "backLeft",		Vec2( -SQRT_2_OVER_2, SQRT_2_OVER_2 ),	spriteAnimSetDefElem );
	AddDirectionAnimation( "back",			Vec2( -1.f, 0.f ),						spriteAnimSetDefElem );
	AddDirectionAnimation( "backRight",		Vec2( -SQRT_2_OVER_2, -SQRT_2_OVER_2 ),	spriteAnimSetDefElem );
	AddDirectionAnimation( "right",			Vec2( 0.f, -1.f ),						spriteAnimSetDefElem );
	AddDirectionAnimation( "frontRight",	Vec2( SQRT_2_OVER_2, -SQRT_2_OVER_2 ),	spriteAnimSetDefElem );
}


//-----------------------------------------------------------------------------------------------
SpriteAnimationSetDefinition::~SpriteAnimationSetDefinition()
{
	PTR_MAP_SAFE_DELETE( m_directionSpriteAnims );
}


//-----------------------------------------------------------------------------------------------
SpriteAnimDefinition* SpriteAnimationSetDefinition::GetSpriteAnimationDefForDirection( const Vec2& entityPos, float entityOrientationDegrees, const Camera& camera )
{
	Vec2 entityToCamera = camera.GetTransform().GetPosition().XY() - entityPos;

	float maxDotProduct = -99999.f;
	std::string closestAnimName = "";
	for ( auto it = m_directionSpriteAnims.begin(); it != m_directionSpriteAnims.end(); ++it )
	{ 
		float dirDotProduct = DotProduct2D( entityToCamera, it->second->facingDirection.GetRotatedDegrees( entityOrientationDegrees ) );

		if ( dirDotProduct > maxDotProduct )
		{
			maxDotProduct = dirDotProduct;
			closestAnimName = it->first;
		}
	}

	auto mapIter = m_directionSpriteAnims.find( closestAnimName );
	if ( mapIter == m_directionSpriteAnims.cend() )
	{
		return nullptr;
	}

	return mapIter->second->animDef;
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimationSetDefinition::AddDirectionAnimation( const std::string& animName, const Vec2& facingDir, const XmlElement& spriteAnimSetDefElem )
{
	if ( spriteAnimSetDefElem.Attribute( animName.c_str() ) )
	{
		Ints spriteIndexes;
		spriteIndexes = ParseXmlAttribute( spriteAnimSetDefElem, animName.c_str(), spriteIndexes );
		if ( spriteIndexes.size() == 0 )
		{
			g_devConsole->PrintError( Stringf( "Animation '%s' %s has no sprite indexes defined.", m_name.c_str(), animName.c_str() ) );
		}
		else
		{
			std::string playbackTypeStr = ParseXmlAttribute( spriteAnimSetDefElem, "playbackType", "" );
			SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::LOOP;
			if ( playbackTypeStr == "once" )
			{
				playbackType = SpriteAnimPlaybackType::ONCE;
			}

			float fps = ParseXmlAttribute( spriteAnimSetDefElem, "fps", 4.f );

			DirectionAnimation* dirAnim = new DirectionAnimation();
			dirAnim->animDef = new SpriteAnimDefinition( *m_spriteSheet, spriteIndexes, fps, playbackType );
			dirAnim->facingDirection = facingDir;

			m_directionSpriteAnims[animName] = dirAnim;
		}
	}
}

#include "Game/EntityDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
EntityDefinition::EntityDefinition( const XmlElement& entityDefElem )
{
	m_name = ParseXmlAttribute( entityDefElem, "name", m_name );
	GUARANTEE_OR_DIE( m_name != "", "Entity did not have a name attribute" );

	m_faction = ParseXmlAttribute( entityDefElem, "faction", m_faction );

	const XmlElement* sizeElement = entityDefElem.FirstChildElement( "Size" );
	if(sizeElement != nullptr)
	{
		m_physicsRadius = ParseXmlAttribute( *sizeElement, "physicsRadius", m_physicsRadius );
		m_localDrawBounds = ParseXmlAttribute( *sizeElement, "localDrawBounds", m_localDrawBounds );
	}

	const XmlElement* healthElement = entityDefElem.FirstChildElement( "Health" );
	if ( healthElement != nullptr )
	{
		m_maxHealth = ParseXmlAttribute( *healthElement, "max", m_maxHealth );
		m_startHealth = ParseXmlAttribute( *healthElement, "start", m_startHealth );
	}

	//<SpriteAnimSet spriteSheet = "SpriteSheets/InventoryItems.png" spriteSheetName = "InventoryItems" spriteLayout = "14,6" fps = "10">
	/*const XmlElement* spriteAnimSetElement = entityDefElem.FirstChildElement( "SpriteAnimSet" );
	if ( spriteAnimSetElement != nullptr )
	{
		IntVec2 spriteLayout;
		std::string spriteLayoutStr = ParseXmlAttribute( *spriteAnimSetElement, "spriteLayout", "0,0" );
		spriteLayout.SetFromText( spriteLayoutStr.c_str() );
		
		std::string spriteSheetPathStr = ParseXmlAttribute( *spriteAnimSetElement, "spriteSheet", "" );

		if ( !spriteSheetPathStr.empty() )
		{
			std::string spriteSheetName = ParseXmlAttribute( *spriteAnimSetElement, "spriteSheetName", "" );
			m_spriteSheet = SpriteSheet::GetSpriteSheet( spriteSheetName );
			if ( m_spriteSheet == nullptr )
			{
				std::string spriteSheetDataPath( "Data/Images/SpriteSheets/" + spriteSheetPathStr );
				SpriteSheet::s_definitions[spriteSheetName] = new SpriteSheet( *( g_renderer->CreateOrGetTextureFromFile( spriteSheetDataPath.c_str() ) ), spriteLayout );
				m_spriteSheet = SpriteSheet::s_definitions[spriteSheetName];
			}
		}

		IntVec2 spriteCoords;
		std::string spriteCoordsStr = ParseXmlAttribute( *spriteAnimSetElement, "spriteCoords", "0,0" );
		spriteCoords.SetFromText( spriteCoordsStr.c_str() );

		if ( m_spriteSheet != nullptr )
		{
			Vec2 mins, maxs;
			m_spriteSheet->GetSpriteUVs( mins, maxs, spriteCoords );
			m_uvCoords.mins = mins;
			m_uvCoords.maxs = maxs;
		}
	}*/
}

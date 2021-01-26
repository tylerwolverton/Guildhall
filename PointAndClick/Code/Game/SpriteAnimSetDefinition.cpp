#include "Game/SpriteAnimSetDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
SpriteAnimSetDefinition::SpriteAnimSetDefinition( RenderContext& renderer, const XmlElement& spriteAnimSetDefElem )
{
	std::string spriteSheetFileName;
	spriteSheetFileName = ParseXmlAttribute( spriteAnimSetDefElem, "spriteSheet", spriteSheetFileName );

	IntVec2 spriteSheetLayout;
	std::string spriteSheetLayoutStr = ParseXmlAttribute( spriteAnimSetDefElem, "spriteLayout", "0,0" );
	spriteSheetLayout.SetFromText( spriteSheetLayoutStr.c_str() );

	// Try to find spritesheet in static map
	std::string spriteSheetName = ParseXmlAttribute( spriteAnimSetDefElem, "spriteSheetName", "" );
	m_spriteSheet = SpriteSheet::GetSpriteSheetByName( spriteSheetName );
	if ( m_spriteSheet == nullptr )
	{
		std::string spriteSheetDataPath( "Data/Images/SpriteSheets/" + spriteSheetFileName );
		m_spriteSheet = new SpriteSheet( *( renderer.CreateOrGetTextureFromFile( spriteSheetDataPath.c_str() ) ), spriteSheetLayout );
		SpriteSheet::s_definitions.push_back( m_spriteSheet );
	}

	if ( spriteSheetFileName == ""
		 || spriteSheetLayout == IntVec2::ZERO )
	{
		// TODO: Should this error and die?
		return;
	}

	float defaultFps = ParseXmlAttribute( spriteAnimSetDefElem, "fps", 1.f );

	//std::string spriteSheetPath( "Data/Images/" + spriteSheetFileName );
	//Texture* spriteSheetTexture = renderer.CreateOrGetTextureFromFile( spriteSheetPath.c_str() );
	//m_spriteSheet = new SpriteSheet( *spriteSheetTexture, spriteSheetLayout );

	// Parse each SpriteAnim
	const XmlElement* spriteAnimElem = spriteAnimSetDefElem.FirstChildElement( "SpriteAnim" );
	while ( spriteAnimElem != nullptr )
	{
		std::string name;
		name = ParseXmlAttribute( *spriteAnimElem, "name", name );
		GUARANTEE_OR_DIE( name != "", "SpriteAnim element did not have a name attribute" );

		Ints spriteIndexes;
		spriteIndexes = ParseXmlAttribute( *spriteAnimElem, "spriteIndexes", spriteIndexes );
		GUARANTEE_OR_DIE( (int)spriteIndexes.size() != 0, Stringf( "No spriteIndexes were defined for animation '%s'.", name.c_str() ) );

		float fps = ParseXmlAttribute( *spriteAnimElem, "fps", defaultFps );

		SpriteAnimDefinition* animDef = new SpriteAnimDefinition( *m_spriteSheet, spriteIndexes, fps );
		m_spriteAnimDefMapByName[name] = animDef;

		spriteAnimElem = spriteAnimElem->NextSiblingElement( "SpriteAnim" );
	}
}


//-----------------------------------------------------------------------------------------------
SpriteAnimSetDefinition::~SpriteAnimSetDefinition()
{
	PTR_SAFE_DELETE( m_spriteSheet );
}

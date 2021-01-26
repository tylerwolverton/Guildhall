#include "Game/SpriteAnimSetDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, SpriteAnimDefinition* > SpriteAnimSetDefinition::s_spriteAnimDefMapByName;


//-----------------------------------------------------------------------------------------------
SpriteAnimSetDefinition::SpriteAnimSetDefinition( RenderContext& renderer, const XmlElement& spriteAnimSetDefElem )
{
	/*<SpriteAnimSet spriteSheet = "KushnariovaCharacters_12x53.png" spriteLayout = "12,53" fps = "10">
		<SpriteAnim name = "MoveEast"		spriteIndexes = "3,4,5,4"/>
		<SpriteAnim name = "MoveWest"		spriteIndexes = "9,10,11,10"/>
		<SpriteAnim name = "MoveNorth"	spriteIndexes = "0,1,2,1"/>
		<SpriteAnim name = "MoveSouth"	spriteIndexes = "6,7,8,7"/>
		<SpriteAnim name = "Idle"				spriteIndexes = "6,7,8,7" fps = "3"/>
		</SpriteAnimSet>*/

	std::string spriteSheetFileName;
	spriteSheetFileName = ParseXmlAttribute( spriteAnimSetDefElem, "spriteSheet", spriteSheetFileName );

	IntVec2 spriteSheetLayout;
	std::string spriteSheetLayoutStr = ParseXmlAttribute( spriteAnimSetDefElem, "spriteLayout", "0,0" );
	spriteSheetLayout.SetFromText( spriteSheetLayoutStr.c_str() );

	// Try to find spritesheet in static map
	m_spriteSheet = SpriteSheet::GetSpriteSheetByName( spriteSheetFileName );
	if ( m_spriteSheet == nullptr )
	{
		std::string spriteSheetDataPath( "Data/Images/" + spriteSheetFileName );
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
		s_spriteAnimDefMapByName[name] = animDef;

		spriteAnimElem = spriteAnimElem->NextSiblingElement( "SpriteAnim" );
	}
}


//-----------------------------------------------------------------------------------------------
SpriteAnimSetDefinition::~SpriteAnimSetDefinition()
{
	PTR_SAFE_DELETE( m_spriteSheet );
}


//-----------------------------------------------------------------------------------------------
SpriteAnimDefinition* SpriteAnimSetDefinition::GetSpriteAnimDefinitionByName( const std::string& name )
{
	std::map< std::string, SpriteAnimDefinition* >::const_iterator  mapIter = s_spriteAnimDefMapByName.find( name );

	if ( mapIter == s_spriteAnimDefMapByName.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


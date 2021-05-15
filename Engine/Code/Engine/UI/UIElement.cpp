#include "Engine/UI/UIElement.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UI/UILabel.hpp"
#include "Engine/UI/UIImage.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/UIText.hpp"


//-----------------------------------------------------------------------------------------------
// Static Definitions
//-----------------------------------------------------------------------------------------------
int UIElement::s_nextId = 100;


//-----------------------------------------------------------------------------------------------
UIElement::UIElement( UISystem& uiSystem, const std::string& name )
	: m_uiSystem( uiSystem )
{
	m_id = GetNextId();
	m_name = name;

	uiSystem.RegisterUIElement( m_id, this );

	if ( !name.empty() )
	{
		uiSystem.RegisterUIElementName( name, m_id );
	}
}


//-----------------------------------------------------------------------------------------------
UIElement::~UIElement()
{
	PTR_SAFE_DELETE( m_userData );
}


//-----------------------------------------------------------------------------------------------
void UIElement::DebugRender() const
{
	if ( !m_isVisible )
	{
		return;
	}

	m_uiSystem.m_renderer->BindTexture( 0, nullptr );
	DrawAABB2Outline( m_uiSystem.m_renderer, m_boundingBox, Rgba8::MAGENTA, UI_DEBUG_LINE_THICKNESS );
}


//-----------------------------------------------------------------------------------------------
void UIElement::SetButtonAndLabelTint( const Rgba8& tint )
{
	m_curTint = tint;

	for ( int labelIdx = 0; labelIdx < (int)m_labels.size(); ++labelIdx )
	{
		m_labels[labelIdx]->SetTint( tint );
	}
}


//-----------------------------------------------------------------------------------------------
Vec2 UIElement::GetBoundingBoxCenter() const
{
	return m_boundingBox.GetCenter();
}


//-----------------------------------------------------------------------------------------------
void UIElement::SetBoundingBoxPercent( const Vec2& percentOfDimensions )
{
	m_boundingBox.maxs.x = m_boundingBox.mins.x + m_initialBoundingBox.GetWidth() * percentOfDimensions.x;
	m_boundingBox.maxs.y = m_boundingBox.mins.y + m_initialBoundingBox.GetHeight() * percentOfDimensions.y;
}


//-----------------------------------------------------------------------------------------------
void UIElement::SetUserData( NamedProperties* userData )
{
	PTR_SAFE_DELETE( m_userData );

	m_userData = userData;
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIElement::AddChildPanel( const UIAlignedPositionData& positionData, Texture* backgroundTexture, const Rgba8& tint,
								   const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	UNUSED( positionData );
	UNUSED( backgroundTexture );
	UNUSED( tint );
	UNUSED( uvAtMins );
	UNUSED( uvAtMaxs );

	g_devConsole->PrintError( "UIElement cannot have a panel added to it" );

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIElement::AddChildPanel( const UIRelativePositionData& positionData, Texture* backgroundTexture, const Rgba8& tint,
								   const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	UNUSED( positionData );
	UNUSED( backgroundTexture );
	UNUSED( tint );
	UNUSED( uvAtMins );
	UNUSED( uvAtMaxs );

	g_devConsole->PrintError( "UIElement cannot have a panel added to it" );

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIElement::AddChildPanel( const std::string& name, const UIAlignedPositionData& positionData,
								   Texture* backgroundTexture, const Rgba8& tint,
						           const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	UNUSED( name );
	UNUSED( positionData );
	UNUSED( backgroundTexture );
	UNUSED( tint );
	UNUSED( uvAtMins );
	UNUSED( uvAtMaxs );

	g_devConsole->PrintError( "UIElement cannot have a panel added to it" );

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
UIPanel* UIElement::AddChildPanel( const std::string& name, const UIRelativePositionData& positionData,
								   Texture* backgroundTexture, const Rgba8& tint,
								   const Vec2& uvAtMins, const Vec2& uvAtMaxs )
{
	UNUSED( name );
	UNUSED( positionData );
	UNUSED( backgroundTexture );
	UNUSED( tint );
	UNUSED( uvAtMins );
	UNUSED( uvAtMaxs );

	g_devConsole->PrintError( "UIElement cannot have a panel added to it" );

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIElement::AddButton( const UIAlignedPositionData& positionData,
							    Texture* backgroundTexture, const Rgba8& tint )
{
	UNUSED( positionData );
	UNUSED( backgroundTexture );
	UNUSED( tint );

	g_devConsole->PrintError( "UIElement cannot have a button added to it" );

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIElement::AddButton( const UIRelativePositionData& positionData,
							    Texture* backgroundTexture, const Rgba8& tint )
{
	UNUSED( positionData );
	UNUSED( backgroundTexture );
	UNUSED( tint );

	g_devConsole->PrintError( "UIElement cannot have a button added to it" );

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIElement::AddButton( const std::string& name, const UIAlignedPositionData& positionData,
							    Texture* backgroundTexture, const Rgba8& tint )
{
	UNUSED( name );
	UNUSED( positionData );
	UNUSED( backgroundTexture );
	UNUSED( tint );
	
	g_devConsole->PrintError( "UIElement cannot have a button added to it" );

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
UIButton* UIElement::AddButton( const std::string& name, const UIRelativePositionData& positionData,
							    Texture* backgroundTexture, const Rgba8& tint )
{
	UNUSED( name );
	UNUSED( positionData );
	UNUSED( backgroundTexture );
	UNUSED( tint );

	g_devConsole->PrintError( "UIElement cannot have a button added to it" );

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
UIUniformGrid* UIElement::AddUniformGrid( const std::string& name, const UIAlignedPositionData& gridPositionData, const IntVec2& gridDimensions, const Vec2& paddingOfGridElementsPixels, Texture* elementTexture, const Rgba8& elementTint )
{
	UNUSED( name );
	UNUSED( gridPositionData );
	UNUSED( gridDimensions );
	UNUSED( paddingOfGridElementsPixels );
	UNUSED( elementTexture );
	UNUSED( elementTint );

	g_devConsole->PrintError( "UIElement cannot have a grid added to it" );

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddImage( const UIAlignedPositionData& positionData, SpriteDefinition* spriteDef )
{
	UILabel* newImage = new UIImage( m_uiSystem, *this, positionData, spriteDef );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddImage( const UIRelativePositionData& positionData, SpriteDefinition* spriteDef )
{
	UILabel* newImage = new UIImage( m_uiSystem, *this, positionData, spriteDef );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddImage( const UIAlignedPositionData& positionData, Texture* image )
{
	UILabel* newImage = new UIImage( m_uiSystem, *this, positionData, image );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddImage( const UIRelativePositionData& positionData, Texture* image )
{
	UILabel* newImage = new UIImage( m_uiSystem, *this, positionData, image );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddImage( const std::string& name, const UIAlignedPositionData& positionData, SpriteDefinition* spriteDef )
{
	UILabel* newImage = new UIImage( m_uiSystem, *this, positionData, spriteDef, name );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddImage( const std::string& name, const UIRelativePositionData& positionData, SpriteDefinition* spriteDef )
{
	UILabel* newImage = new UIImage( m_uiSystem, *this, positionData, spriteDef, name );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddImage( const std::string& name, const UIAlignedPositionData& positionData, Texture* image )
{
	UILabel* newImage = new UIImage( m_uiSystem, *this, positionData, image, name );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddImage( const std::string& name, const UIRelativePositionData& positionData, Texture* image )
{
	UILabel* newImage = new UIImage( m_uiSystem, *this, positionData, image, name );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddText( const UIAlignedPositionData& positionData, const std::string& text, float fontSize, const Vec2& textAlignment )
{
	UILabel* newText = new UIText( m_uiSystem, *this, positionData, text, fontSize, textAlignment );
	m_labels.push_back( newText );

	return newText;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddText( const UIRelativePositionData& positionData, const std::string& text, float fontSize, const Vec2& textAlignment )
{
	UILabel* newText = new UIText( m_uiSystem, *this, positionData, text, fontSize, textAlignment );
	m_labels.push_back( newText );

	return newText;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddText( const std::string& name, const UIAlignedPositionData& positionData, const std::string& text, float fontSize, const Vec2& textAlignment )
{
	UILabel* newText = new UIText( m_uiSystem, *this, positionData, text, fontSize, textAlignment, name );
	m_labels.push_back( newText );

	return newText;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddText( const std::string& name, const UIRelativePositionData& positionData, const std::string& text, float fontSize, const Vec2& textAlignment )
{
	UILabel* newText = new UIText( m_uiSystem, *this, positionData, text, fontSize, textAlignment, name );
	m_labels.push_back( newText );

	return newText;
}


//-----------------------------------------------------------------------------------------------
void UIElement::ClearLabels()
{
	PTR_VECTOR_SAFE_DELETE( m_labels );
}


//-----------------------------------------------------------------------------------------------
// Static Methods
//-----------------------------------------------------------------------------------------------
uint UIElement::GetNextId()
{
	uint nextId = s_nextId;
	++s_nextId;

	return nextId;
}

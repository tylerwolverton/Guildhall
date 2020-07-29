#include "Engine/UI/UIElement.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UI/UILabel.hpp"
#include "Engine/UI/UIImage.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/UI/UIText.hpp"


//-----------------------------------------------------------------------------------------------
// Static Definitions
//-----------------------------------------------------------------------------------------------
uint UIElement::s_nextId = 0;


//-----------------------------------------------------------------------------------------------
UIElement::UIElement( const UISystem& uiSystem )
	: m_uiSystem( uiSystem )
{

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
	m_tint = tint;

	for ( int labelIdx = 0; labelIdx < (int)m_labels.size(); ++labelIdx )
	{
		m_labels[labelIdx]->SetTint( tint );
	}
}


//-----------------------------------------------------------------------------------------------
void UIElement::SetUserData( NamedProperties* userData )
{
	PTR_SAFE_DELETE( m_userData );

	m_userData = userData;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddImage( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, Texture* image )
{
	UILabel* newImage = new UIImage( m_uiSystem, *this, relativeFractionMinPosition, relativeFractionOfDimensions, image );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddImage( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, SpriteDefinition* spriteDef )
{
	UILabel* newImage = new UIImage( m_uiSystem, *this, relativeFractionMinPosition, relativeFractionOfDimensions, spriteDef );
	m_labels.push_back( newImage );

	return newImage;
}


//-----------------------------------------------------------------------------------------------
UILabel* UIElement::AddText( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions, const std::string& text, float fontSize, const Vec2& alignment )
{
	UILabel* newText = new UIText( m_uiSystem, *this, relativeFractionMinPosition, relativeFractionOfDimensions, text, fontSize, alignment );
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

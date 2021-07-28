#include "Engine/UI/UISystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UI/UIPanel.hpp"
#include "Engine/UI/UIButton.hpp"
#include "Engine/UI/UIUniformGrid.hpp"
#include "Engine/UI/UIImage.hpp"
#include "Engine/UI/UIText.hpp"


//-----------------------------------------------------------------------------------------------
void UISystem::Startup( Window* window, RenderContext* renderer, BitmapFont* font, Material* material )
{
	m_renderer = renderer;
	m_inputSystem = window->GetInputSystem();
	m_windowDimensions = window->GetDimensions();

	if ( font == nullptr )
	{
		m_uiFont = m_renderer->GetSystemFont();
	}
	else
	{
		m_uiFont = font;
		m_uiFontMaterial = material;
	}
		
	m_rootPanel = new UIPanel( *this, AABB2( Vec2::ZERO, m_windowDimensions ) );
}


//-----------------------------------------------------------------------------------------------
void UISystem::Startup( const Vec2& windowDimensions, RenderContext* renderer )
{
	m_renderer = renderer;
	m_windowDimensions = windowDimensions;
	m_uiFont = m_renderer->GetSystemFont();

	m_rootPanel = new UIPanel( *this, AABB2( Vec2::ZERO, m_windowDimensions ) );
}


//-----------------------------------------------------------------------------------------------
void UISystem::Update()
{
	m_rootPanel->Update();
}


//-----------------------------------------------------------------------------------------------
void UISystem::Render()
{
	m_rootPanel->Render();
}


//-----------------------------------------------------------------------------------------------
void UISystem::DebugRender()
{
	m_rootPanel->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void UISystem::Shutdown()
{
	PTR_SAFE_DELETE( m_rootPanel );
}


//-----------------------------------------------------------------------------------------------
AABB2 UISystem::GetBoundingBoxFromParentAndPositionData( const AABB2& parentBoundingBox, const UIAlignedPositionData& positionData ) const
{
	float width = parentBoundingBox.GetWidth() * positionData.fractionOfParentDimensions.x;
	float height = parentBoundingBox.GetHeight() * positionData.fractionOfParentDimensions.y;

	AABB2 boundingBox = parentBoundingBox;
	boundingBox.mins = parentBoundingBox.mins;
	boundingBox.maxs = parentBoundingBox.mins + Vec2( width, height );

	boundingBox.AlignWithinBounds( parentBoundingBox, positionData.alignmentWithinParentElement );

	PadBoundingBox( boundingBox, positionData.horizontalPaddingInPixels, positionData.verticalPaddingInPixels );
	OffsetBoundingBox( boundingBox, parentBoundingBox, positionData.positionOffsetFraction, positionData.positionOffsetInPixels );

	return boundingBox;
}


//-----------------------------------------------------------------------------------------------
AABB2 UISystem::GetBoundingBoxFromParentAndPositionData( const AABB2& parentBoundingBox, const UIRelativePositionData& positionData ) const
{
	// Set position based on parent and specified dimension fractions
	AABB2 boundingBox = parentBoundingBox;
	float width = parentBoundingBox.GetWidth();
	float height = parentBoundingBox.GetHeight();

	boundingBox.mins = Vec2( boundingBox.mins.x + positionData.widthFractionRange.x * width,
							 boundingBox.mins.y + positionData.heightFractionRange.x * height );

	boundingBox.maxs = Vec2( boundingBox.mins.x + positionData.widthFractionRange.y * width,
							 boundingBox.mins.y + positionData.heightFractionRange.y * height );

	PadBoundingBox( boundingBox, positionData.horizontalPaddingInPixels, positionData.verticalPaddingInPixels );
	OffsetBoundingBox( boundingBox, parentBoundingBox, positionData.positionOffsetFraction, positionData.positionOffsetInPixels );

	return boundingBox;
}


//-----------------------------------------------------------------------------------------------
void UISystem::PadBoundingBox( AABB2& boundingBox, const Vec2& horizontalPaddingInPixels, const Vec2& verticalPaddingInPixels ) const
{
	boundingBox.mins.x += horizontalPaddingInPixels.x;
	boundingBox.maxs.x -= horizontalPaddingInPixels.y;

	boundingBox.mins.y += verticalPaddingInPixels.x;
	boundingBox.maxs.y -= verticalPaddingInPixels.y;
}


//-----------------------------------------------------------------------------------------------
void UISystem::OffsetBoundingBox( AABB2& boundingBox, const AABB2& parentBoundingBox, const Vec2& offsetFraction, const Vec2& offsetInPixels ) const
{
	Vec2 relativeOffset = offsetFraction * Vec2( parentBoundingBox.GetWidth(), parentBoundingBox.GetHeight() );
	boundingBox.mins += relativeOffset;
	boundingBox.maxs += relativeOffset;

	boundingBox.mins += offsetInPixels;
	boundingBox.maxs += offsetInPixels;
}


//-----------------------------------------------------------------------------------------------
void UISystem::RegisterUIElement( uint id, UIElement* uiElement )
{
	m_elementsById[id] = uiElement;
}


//-----------------------------------------------------------------------------------------------
void UISystem::RegisterUIElementName( const std::string& name, uint id )
{
	m_elementNameToId[name] = id;
}


//-----------------------------------------------------------------------------------------------
void UISystem::LoadUIElementsFromXML( const XmlElement& uiRootElem )
{
	if ( strcmp( uiRootElem.Name(), "UIRoot" ) )
	{
		g_devConsole->PrintError( "Must use Root as the root element of XML file with other elements as children" );
		return;
	}

	const XmlElement* uiElementElem = uiRootElem.FirstChildElement( "UIElement" );
	while ( uiElementElem != nullptr )
	{
		ParseUIElementXml( *uiElementElem, m_rootPanel );

		uiElementElem = uiElementElem->NextSiblingElement();
	}
}



//-----------------------------------------------------------------------------------------------
UIElement* UISystem::GetUIElementByName( const std::string& elemName )
{
	auto idIter = m_elementNameToId.find( elemName );
	if ( idIter == m_elementNameToId.end() )
	{
		return nullptr;
	}
	
	auto elemIter = m_elementsById.find( idIter->second );
	if ( elemIter == m_elementsById.end() )
	{
		return nullptr;
	}

	return elemIter->second;
}


//-----------------------------------------------------------------------------------------------
UIElement* UISystem::GetUIElementById( int elemId )
{
	auto elemIter = m_elementsById.find( elemId );
	if ( elemIter == m_elementsById.end() )
	{
		return nullptr;
	}

	return elemIter->second;
}


//-----------------------------------------------------------------------------------------------
bool UISystem::IsMouseCursorInUI() const
{
	for ( int elemIdx = 0; elemIdx < (int)m_rootPanel->m_childPanels.size(); ++elemIdx )
	{
		if( m_rootPanel->m_childPanels[elemIdx]->IsActive()
			&& m_rootPanel->m_childPanels[elemIdx]->GetBoundingBox().IsPointInside( m_inputSystem->GetNormalizedMouseClientPos() * m_windowDimensions ) )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void UISystem::SetFont( BitmapFont* font, Material* material )
{
	if ( font == nullptr )
	{
		g_devConsole->PrintError( "Tried to set a null font in UI system" );
		return;
	}

	m_uiFont = font;
	m_uiFontMaterial = material;
}

//-----------------------------------------------------------------------------------------------
void UISystem::ParseUIElementXml( const XmlElement& uiElementElem, UIElement* parentElem )
{
	// Top level attributes
	std::string name = ParseXmlAttribute( uiElementElem, "name", "" );
	if ( name == "" )
	{
		g_devConsole->PrintError( "UI.xml: UI Element is missing a name attribute" );
		return;
	}

	std::string elementType = ParseXmlAttribute( uiElementElem, "type", "" );
	if ( elementType == "" )
	{
		g_devConsole->PrintError( Stringf( "UI.xml: UI Element '%s' is missing a type attribute", name.c_str() ) );
		return;
	}

	Rgba8 tint = ParseXmlAttribute( uiElementElem, "tint", Rgba8::WHITE );
	Rgba8 hoverTint = ParseXmlAttribute( uiElementElem, "hoverTint", Rgba8::WHITE );

	std::string texturePath = ParseXmlAttribute( uiElementElem, "texture", "" );
	Texture* texture = nullptr;
	if ( !texturePath.empty() )
	{
		if ( IsEqualIgnoreCase( texturePath, "white" ) )
		{
			texture = m_renderer->GetDefaultWhiteTexture();
		}
		else
		{
			texture = m_renderer->CreateOrGetTextureFromFile( texturePath.c_str() );
		}
		
		if ( texture == nullptr )
		{
			texture = m_renderer->GetDefaultWhiteTexture();
		}
	}

	bool isHidden = ParseXmlAttribute( uiElementElem, "isHidden", false );
	bool isActive = ParseXmlAttribute( uiElementElem, "isActive", true );

	// Parse position data
	const XmlElement* positionDataElem = uiElementElem.FirstChildElement( "PositionData" );
	if ( positionDataElem == nullptr )
	{
		g_devConsole->PrintError( "UI.xml: UI Element is missing a PositionData node" );
		return;
	}

	std::string posType = ParseXmlAttribute( *positionDataElem, "type", "" );
	if ( posType.empty() )
	{
		g_devConsole->PrintError( "UI.xml: UI Element PositionData is missing a type attribute. Valid types are: aligned and relative" );
		return;
	}

	UIElement* newElement = nullptr;

	/*if ( IsEqualIgnoreCase( posType, "aligned" ) )
	{*/
		UIAlignedPositionData posData = ParseAlignedPositionData( *positionDataElem );

		if ( IsEqualIgnoreCase( elementType, "panel" ) )
		{
			newElement = parentElem->AddChildPanel( name, posData, texture, tint );
		}
		else if ( IsEqualIgnoreCase( elementType, "button" ) )
		{
			newElement = parentElem->AddButton( name, posData, texture, tint );
		}
		else if ( IsEqualIgnoreCase( elementType, "image" ) )
		{
			newElement = parentElem->AddImage( name, posData, texture );
		}
		else if ( IsEqualIgnoreCase( elementType, "text" ) )
		{
			std::string text = ParseXmlAttribute( uiElementElem, "text", "" );
			float fontSize = ParseXmlAttribute( uiElementElem, "fontSize", 24.f );

			newElement = parentElem->AddText( name, posData, text, fontSize, ALIGN_CENTERED_LEFT );
		}
		else if ( IsEqualIgnoreCase( elementType, "grid" ) )
		{
			IntVec2 dimensions = ParseXmlAttribute( uiElementElem, "dimensions", IntVec2( 1, 1 ) );
			Vec2 elementPaddingPixels = ParseXmlAttribute( uiElementElem, "elementPaddingPixels", Vec2::ZERO );

			newElement = parentElem->AddUniformGrid( name, posData, dimensions, elementPaddingPixels, m_renderer->GetDefaultWhiteTexture(), tint );
		}
	/*}
	else
	{
		g_devConsole->PrintError( "UI.xml: Unsupported PositionData type attribute. Valid types are: aligned and relative" );
		return;
	}*/

	if ( newElement == nullptr )
	{
		return;
	}

	newElement->SetHoverTint( hoverTint );
	isHidden ? newElement->Hide() : newElement->Show();
	isActive ? newElement->Activate() : newElement->Deactivate();

	// Recursively parse child nodes
	const XmlElement* childUIElementElem = uiElementElem.FirstChildElement( "UIElement" );
	while ( childUIElementElem != nullptr )
	{
		ParseUIElementXml( *childUIElementElem, newElement );

		childUIElementElem = childUIElementElem->NextSiblingElement();
	}
	
}


//-----------------------------------------------------------------------------------------------
UIAlignedPositionData UISystem::ParseAlignedPositionData( const XmlElement& positionDataElem )
{
	/*<PositionData type = "aligned"
		fractionOfParentDimensions = ".5,.5"
		horizontalPaddingInPixels = "0,0"
		verticalPaddingInPixels = "0,0"
		positionOffsetFraction = "0,0"
		positionOffsetInPixels = "0,0"
		alignmentWithinParentElement = "center"/>*/

	UIAlignedPositionData posData;

	posData.fractionOfParentDimensions =	ParseXmlAttribute( positionDataElem, "fractionOfParentDimensions",	posData.fractionOfParentDimensions );
	posData.horizontalPaddingInPixels =		ParseXmlAttribute( positionDataElem, "horizontalPaddingInPixels",	posData.horizontalPaddingInPixels );
	posData.verticalPaddingInPixels =		ParseXmlAttribute( positionDataElem, "verticalPaddingInPixels",		posData.verticalPaddingInPixels );
	posData.positionOffsetFraction =		ParseXmlAttribute( positionDataElem, "positionOffsetFraction",		posData.positionOffsetFraction );
	posData.positionOffsetInPixels =		ParseXmlAttribute( positionDataElem, "positionOffsetInPixels",		posData.positionOffsetInPixels );

	// Convert to alignment type
	std::string alignmentWithinParentElementStr = ParseXmlAttribute( positionDataElem, "alignmentWithinParentElement", "center" );
	
	if ( IsEqualIgnoreCase( alignmentWithinParentElementStr, "center" ) )
	{
		posData.alignmentWithinParentElement = ALIGN_CENTERED;
	}
	else if ( IsEqualIgnoreCase( alignmentWithinParentElementStr, "center-left" ) )
	{
		posData.alignmentWithinParentElement = ALIGN_CENTERED_LEFT;
	}
	else if ( IsEqualIgnoreCase( alignmentWithinParentElementStr, "center-right" ) )
	{
		posData.alignmentWithinParentElement = ALIGN_CENTERED_RIGHT;
	}
	else if ( IsEqualIgnoreCase( alignmentWithinParentElementStr, "bottom-left" ) )
	{
		posData.alignmentWithinParentElement = ALIGN_BOTTOM_LEFT;
	}
	else if ( IsEqualIgnoreCase( alignmentWithinParentElementStr, "bottom-center" ) )
	{
		posData.alignmentWithinParentElement = ALIGN_BOTTOM_CENTER;
	}
	else if ( IsEqualIgnoreCase( alignmentWithinParentElementStr, "bottom-right" ) )
	{
		posData.alignmentWithinParentElement = ALIGN_BOTTOM_RIGHT;
	}
	else if ( IsEqualIgnoreCase( alignmentWithinParentElementStr, "top-left" ) )
	{
		posData.alignmentWithinParentElement = ALIGN_TOP_LEFT;
	}
	else if ( IsEqualIgnoreCase( alignmentWithinParentElementStr, "top-center" ) )
	{
		posData.alignmentWithinParentElement = ALIGN_TOP_CENTER;
	}
	else if ( IsEqualIgnoreCase( alignmentWithinParentElementStr, "top-right" ) )
	{
		posData.alignmentWithinParentElement = ALIGN_TOP_RIGHT;
	}
	else
	{
		g_devConsole->PrintError( "UI.xml: Unsupported alignmentWithinParentElement attribute. Valid types are: center, center-left, center-right, bottom-left, bottom-center, bottom-right, top-left, top-center, and top-right" );
	}

	return posData;
}


////-----------------------------------------------------------------------------------------------
//UIRelativePositionData UISystem::ParseRelativePositionData( const XmlElement& positionDataElem )
//{
//
//	UIRelativePositionData posData;
//	return posData;
//}


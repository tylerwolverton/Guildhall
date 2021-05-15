#include "Engine/UI/UIUniformGrid.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UI/UIPanel.hpp"
#include "Engine/UI/UIButton.hpp"
#include "Engine/UI/UISystem.hpp"

#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
UIUniformGrid::UIUniformGrid( UISystem& uiSystem, const UIElement& parentPanel, const std::string& name,
							  const UIAlignedPositionData& gridPositionData, const IntVec2& gridDimensions, const Vec2& paddingOfGridElementsPixels, 
							  Texture* elementTexture, const Rgba8& elementTint )
	: UIElement( uiSystem, name )
{
	m_gridCapacity = gridDimensions.x * gridDimensions.y;

	m_boundingBox = uiSystem.GetBoundingBoxFromParentAndPositionData( parentPanel.GetBoundingBox(), gridPositionData );
	m_initialBoundingBox = m_boundingBox;

	float elemWidth =  ( m_boundingBox.GetWidth()  - ( (float)( gridDimensions.x + 1 ) * paddingOfGridElementsPixels.x ) ) / (float)gridDimensions.x;
	float elemHeight = ( m_boundingBox.GetHeight() - ( (float)( gridDimensions.y + 1 ) * paddingOfGridElementsPixels.y ) ) / (float)gridDimensions.y;

	Vec2 elemDimensions( elemWidth, elemHeight );

	Vec2 nextElemPos( m_boundingBox.mins.x + paddingOfGridElementsPixels.x, m_boundingBox.maxs.y - paddingOfGridElementsPixels.y - elemHeight );

	for ( int elemYIdx = 0; elemYIdx < gridDimensions.y; ++elemYIdx )
	{
		nextElemPos.x = m_boundingBox.mins.x + paddingOfGridElementsPixels.x;

		for ( int elemXIdx = 0; elemXIdx < gridDimensions.x; ++elemXIdx )
		{
			Vec2 elementMins = nextElemPos;
			Vec2 elementMaxs = nextElemPos + elemDimensions;

			nextElemPos.x += elemWidth + paddingOfGridElementsPixels.x;

			AABB2 elementAbsoluteBounds( elementMins, elementMaxs );

			UIButton* newButton = new UIButton( uiSystem, elementAbsoluteBounds, elementTexture, elementTint, name + ToString( elemXIdx ) + ToString( elemYIdx ) );
			
			newButton->SetHoverTint( elementTint + Rgba8( 50, 50, 50, 0 ) );
			newButton->m_onHoverBeginEvent.SubscribeMethod( this, &UIUniformGrid::OnElementHoverBegin );
			newButton->m_onHoverEndEvent.SubscribeMethod( this, &UIUniformGrid::OnElementHoverEnd );
			
			m_gridElements.push_back( newButton );
		}

		nextElemPos.y -= ( elemHeight + paddingOfGridElementsPixels.y );
	}
}


//-----------------------------------------------------------------------------------------------
void UIUniformGrid::Update()
{
	if ( !IsActive() )
	{
		return;
	}

	for ( int elemIdx = 0; elemIdx < (int)m_gridElements.size(); ++elemIdx )
	{
		m_gridElements[elemIdx]->Update();
	}
}


//-----------------------------------------------------------------------------------------------
void UIUniformGrid::Render() const
{
	if ( !IsVisible() )
	{
		return;
	}

	for ( int elemIdx = 0; elemIdx < (int)m_gridElements.size(); ++elemIdx )
	{
		m_gridElements[elemIdx]->Render();
	}
}


//-----------------------------------------------------------------------------------------------
void UIUniformGrid::DebugRender() const
{
	if ( !IsVisible() )
	{
		return;
	}

	for ( int elemIdx = 0; elemIdx < (int)m_gridElements.size(); ++elemIdx )
	{
		m_gridElements[elemIdx]->DebugRender();
	}
}


//-----------------------------------------------------------------------------------------------
UIUniformGrid::~UIUniformGrid()
{
	PTR_VECTOR_SAFE_DELETE( m_gridElements );
}


//-----------------------------------------------------------------------------------------------
void UIUniformGrid::OnElementHoverBegin( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	button->ActivateHoverTint();
}


//-----------------------------------------------------------------------------------------------
void UIUniformGrid::OnElementHoverEnd( EventArgs* args )
{
	UIButton* button = (UIButton*)args->GetValue( "button", ( void* )nullptr );
	if ( button == nullptr )
	{
		return;
	}

	button->ResetTint();
}


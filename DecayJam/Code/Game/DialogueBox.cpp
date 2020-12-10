#include "Game/DialogueBox.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/UI/UIPanel.hpp"
#include "Engine/UI/UILabel.hpp"

#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
DialogueBox::DialogueBox( UISystem& uiSystem, const UIAlignedPositionData& positionData )
{
	m_dialogueRootPanel = uiSystem.GetRootPanel()->AddChildPanel( positionData, g_renderer->GetDefaultWhiteTexture(), Rgba8::BLACK );
	m_dialogueRootPanel->Deactivate();
	m_dialogueRootPanel->Hide();

	m_firstLinePosData.fractionOfParentDimensions = Vec2( .8f, .3f );
	m_firstLinePosData.positionOffsetFraction = Vec2( .05f, -.05f );
	m_firstLinePosData.alignmentWithinParentElement = ALIGN_TOP_LEFT;

	m_curLinePosData = m_firstLinePosData;

	m_selectorVertices[0] = Vertex_PCU( Vec2::ZERO,			Rgba8::WHITE );
	m_selectorVertices[1] = Vertex_PCU( Vec2( .75f, .5f ),	Rgba8::WHITE );
	m_selectorVertices[2] = Vertex_PCU( Vec2( 0.f, 1.f ),	Rgba8::WHITE );
}


//-----------------------------------------------------------------------------------------------
void DialogueBox::Update()
{
	if ( !m_dialogueRootPanel->IsActive() )
	{
		return;
	}

	if ( g_inputSystem->WasKeyJustPressed( 'W' )
		 || g_inputSystem->WasKeyJustPressed( KEY_UPARROW ) )
	{
		--m_curChoiceIdx;
		if ( m_curChoiceIdx < 0 )
		{
			m_curChoiceIdx = (int)m_choicePositions.size() - 1;
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( 'S' )
		 || g_inputSystem->WasKeyJustPressed( KEY_DOWNARROW ) )
	{
		++m_curChoiceIdx;
		if ( m_curChoiceIdx > (int)m_choicePositions.size() - 1 )
		{
			m_curChoiceIdx = 0;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void DialogueBox::Render() const
{
	if ( !m_dialogueRootPanel->IsVisible() )
	{
		return;
	}

	Vertex_PCU vertexCopy[3];
	for ( int vertexIdx = 0; vertexIdx < 3; ++vertexIdx )
	{
		vertexCopy[vertexIdx] = m_selectorVertices[vertexIdx];
	}
	
	AABB2 dialogueBounds = m_dialogueRootPanel->GetBoundingBox();
	if ( m_choicePositions.size() == 0 )
	{
		Vertex_PCU::TransformVertexArray( vertexCopy, 3, 20.f, -90.f, Vec3( dialogueBounds.maxs.x - 30.f, dialogueBounds.mins.y + 25.f, 0.f ) );
	}
	else
	{
		Vec3 translation = Vec3( m_choicePositions[m_curChoiceIdx], 0.f );
		Vertex_PCU::TransformVertexArray( vertexCopy, 3, 20.f, 0.f, translation );
	}

	g_renderer->BindTexture( 0, nullptr );
	g_renderer->DrawVertexArray( 3, vertexCopy );
}


//-----------------------------------------------------------------------------------------------
void DialogueBox::Show()
{
	if ( m_dialogueRootPanel == nullptr )
	{
		return;
	}

	m_dialogueRootPanel->Show();
	m_dialogueRootPanel->Activate();
}


//-----------------------------------------------------------------------------------------------
void DialogueBox::Hide()
{
	if ( m_dialogueRootPanel == nullptr )
	{
		return;
	}

	m_dialogueRootPanel->Deactivate();
	m_dialogueRootPanel->Hide();
}


//-----------------------------------------------------------------------------------------------
void DialogueBox::Clear()
{
	if ( m_dialogueRootPanel == nullptr )
	{
		return;
	}

	m_dialogueRootPanel->ClearLabels();
	m_curLinePosData = m_firstLinePosData;

	m_choicePositions.clear();
	m_choiceNames.clear();
	m_curChoiceIdx = 0;
}


//-----------------------------------------------------------------------------------------------
void DialogueBox::Reset()
{
	if ( m_dialogueRootPanel == nullptr )
	{
		return;
	}

	Hide();

	m_dialogueRootPanel->ClearLabels();
	m_curLinePosData = m_firstLinePosData;

	m_choicePositions.clear();
	m_choiceNames.clear();
	m_curChoiceIdx = 0;
}


//-----------------------------------------------------------------------------------------------
std::string DialogueBox::GetCurrentChoiceName() const
{
	if ( m_choiceNames.size() == 0 )
	{
		return "";
	}

	return m_choiceNames[m_curChoiceIdx];
}


//-----------------------------------------------------------------------------------------------
void DialogueBox::AddLineOfText( const std::string& text )
{
	if ( m_dialogueRootPanel == nullptr )
	{
		return;
	}

	m_dialogueRootPanel->AddText( m_curLinePosData, text, 24.f, ALIGN_TOP_LEFT );

	m_curLinePosData.positionOffsetFraction += Vec2( .0f, -.1f );
}


//-----------------------------------------------------------------------------------------------
void DialogueBox::AddChoice( const std::string& name, const std::string& text )
{
	if ( m_dialogueRootPanel == nullptr )
	{
		return;
	}

	// Add blank line between text and choices
	if ( m_choicePositions.size() == 0 )
	{
		m_curLinePosData.positionOffsetFraction += Vec2( .01f, -.1f );
	}

	UILabel* choice = m_dialogueRootPanel->AddText( m_curLinePosData, text, 20.f, ALIGN_TOP_LEFT );
	
	m_choiceNames.push_back( name );

	// Save position to the left of choice to use with selector
	AABB2 choiceBounds = choice->GetBounds();
	Vec2 choiceEdge( choiceBounds.mins.x - 25.f, choiceBounds.maxs.y - 19.f );
	m_choicePositions.push_back( choiceEdge );

	m_curLinePosData.positionOffsetFraction += Vec2( .0f, -.09f );
}

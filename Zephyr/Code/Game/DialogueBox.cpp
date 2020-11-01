#include "Game/DialogueBox.hpp"
#include "Engine/Core/Rgba8.hpp"
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
void DialogueBox::Reset()
{
	if ( m_dialogueRootPanel == nullptr )
	{
		return;
	}

	Hide();

	m_dialogueRootPanel->ClearLabels();
	m_curLinePosData = m_firstLinePosData;
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

	Show();
}

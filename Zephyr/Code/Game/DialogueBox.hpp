#pragma once
#include "Engine/UI/UISystem.hpp"


//-----------------------------------------------------------------------------------------------
class UIPanel;


//-----------------------------------------------------------------------------------------------
class DialogueBox
{
public:
	DialogueBox( UISystem& uiSystem, const UIAlignedPositionData& positionData );

	void Show();
	void Hide();
	void Reset();

	void AddLineOfText( const std::string& text );

private:
	UIPanel* m_dialogueRootPanel = nullptr;
	UIAlignedPositionData m_firstLinePosData;
	UIAlignedPositionData m_curLinePosData;
};

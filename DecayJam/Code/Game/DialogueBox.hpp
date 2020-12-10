#pragma once
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/UI/UISystem.hpp"


//-----------------------------------------------------------------------------------------------
class UIPanel;


//-----------------------------------------------------------------------------------------------
class DialogueBox
{
public:
	DialogueBox( UISystem& uiSystem, const UIAlignedPositionData& positionData );

	void Update();
	void Render() const;

	void Show();
	void Hide();
	void Clear();
	void Reset();

	std::string GetCurrentChoiceName() const;

	void AddLineOfText( const std::string& text );
	void AddChoice( const std::string& name, const std::string& text );

private:
	UIPanel*				 m_dialogueRootPanel = nullptr;
	UIAlignedPositionData	 m_firstLinePosData;
	UIAlignedPositionData	 m_curLinePosData;

	std::vector<std::string> m_choiceNames;
	std::vector<Vec2>		 m_choicePositions;
	int						 m_curChoiceIdx = 0;

	Vertex_PCU				 m_selectorVertices[3];
};

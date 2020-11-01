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
	void Reset();

	void AddLineOfText( const std::string& text );
	void AddChoice( const std::string& text );

private:
	UIPanel*				m_dialogueRootPanel = nullptr;
	UIAlignedPositionData	m_firstLinePosData;
	UIAlignedPositionData	m_curLinePosData;

	std::vector<Vec2>		m_choicePositions;
	Vec2					m_curChoicePos;

	Vertex_PCU				m_selectorVertices[3];
};

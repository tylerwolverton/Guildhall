#include "Game/UI/UILabel.hpp"
#include "Game/UI/UIButton.hpp"


//-----------------------------------------------------------------------------------------------
UILabel::UILabel( const UIButton& parentButton, const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions )
{
	AABB2 boundingBox = parentButton.GetBoundingBox();
	float width = boundingBox.GetWidth();
	float height = boundingBox.GetHeight();

	m_boundingBox.mins = Vec2( boundingBox.mins.x + relativeFractionMinPosition.x * width,
							   boundingBox.mins.y + relativeFractionMinPosition.y * height );

	m_boundingBox.maxs = Vec2( m_boundingBox.mins.x + relativeFractionOfDimensions.x * width,
							   m_boundingBox.mins.y + relativeFractionOfDimensions.y * height );
}

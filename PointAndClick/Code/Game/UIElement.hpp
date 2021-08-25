#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
class RenderContext;
class SpriteDefinition;
class UILabel;
class Texture;


//-----------------------------------------------------------------------------------------------
class UIElement
{
public:
	UIElement() = default;
	~UIElement();

	virtual void Update() = 0;
	virtual void Render( RenderContext* renderer ) const = 0;
	virtual void DebugRender( RenderContext* renderer ) const;

	virtual void Activate()													{ m_isActive = true; }
	virtual void Deactivate()												{ m_isActive = false; }
	virtual void Hide()														{ m_isVisible = false; }
	virtual void Show()														{ m_isVisible = true; }

	uint GetId() const														{ return m_id; }
	void SetBackgroundTexture( Texture* backgroundTexture )					{ m_backgroundTexture = backgroundTexture; }
	void SetButtonTint( const Rgba8& tint )									{ m_tint = tint; }
	void SetButtonAndLabelTint( const Rgba8& tint );
	AABB2 GetBoundingBox() const											{ return m_boundingBox; }

	NamedProperties* GetUserData() const									{ return m_userData; }
	void SetUserData( NamedProperties* userData );

	UILabel* AddImage( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions,
					   Texture* image = nullptr );
	UILabel* AddImage( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions,
					   SpriteDefinition* spriteDef = nullptr );
	UILabel* AddText( const Vec2& relativeFractionMinPosition, const Vec2& relativeFractionOfDimensions,
					  const std::string& text, float fontSize = 24.f, const Vec2& alignment = ALIGN_CENTERED );

	void	 ClearLabels();

	// Static methods
	static uint GetNextId();

protected:
	uint m_id = 0;
	NamedProperties* m_userData = nullptr;
	bool m_isActive = true;
	bool m_isVisible = true;

	AABB2 m_boundingBox;
	Rgba8 m_tint = Rgba8::WHITE;

	Texture* m_backgroundTexture = nullptr;

	std::vector<UILabel*> m_labels;

private:
	static uint s_nextId;
};
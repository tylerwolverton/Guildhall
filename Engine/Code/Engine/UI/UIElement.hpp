#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
struct UIAlignedPositionData;
struct UIRelativePositionData;
class SpriteDefinition;
class UILabel;
class UISystem;
class Texture;


//-----------------------------------------------------------------------------------------------
class UIElement
{
public:
	UIElement( const UISystem& uiSystem );
	virtual ~UIElement();

	virtual void Update() = 0;
	virtual void Render() const = 0;
	virtual void DebugRender() const = 0;

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

	// Add methods for labels
	UILabel* AddImage( const UIAlignedPositionData& positionData, Texture* image = nullptr );
	UILabel* AddImage( const UIRelativePositionData& positionData, Texture* image = nullptr );
	UILabel* AddImage( const UIAlignedPositionData& positionData, SpriteDefinition* spriteDef = nullptr );
	UILabel* AddImage( const UIRelativePositionData& positionData, SpriteDefinition* spriteDef = nullptr );
	UILabel* AddText( const UIAlignedPositionData& positionData, const std::string& text, float fontSize = 24.f, const Vec2& textAlignment = ALIGN_CENTERED );
	UILabel* AddText( const UIRelativePositionData& positionData, const std::string& text, float fontSize = 24.f, const Vec2& textAlignment = ALIGN_CENTERED );

	void	 ClearLabels();

	// Static methods
	static uint GetNextId();

protected:
	const UISystem& m_uiSystem;

	uint m_id = 0;
	NamedProperties* m_userData = nullptr;
	bool m_isActive = true;
	bool m_isVisible = true;

	AABB2 m_boundingBox;
	Rgba8 m_tint = Rgba8::WHITE;
	Vec2 m_uvsAtMins = Vec2::ZERO;
	Vec2 m_uvsAtMaxs = Vec2::ONE;

	Texture* m_backgroundTexture = nullptr;

	std::vector<UILabel*> m_labels;

private:
	static uint s_nextId;
};

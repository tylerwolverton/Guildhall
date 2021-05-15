#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
struct UIAlignedPositionData;
struct UIRelativePositionData;
class SpriteDefinition;
class UILabel;
class UIButton;
class UIUniformGrid;
class UIPanel;
class UISystem;
class Texture;


//-----------------------------------------------------------------------------------------------
class UIElement
{
	friend class UISystem;

public:
	virtual ~UIElement();

	virtual void Update() = 0;
	virtual void Render() const = 0;
	virtual void DebugRender() const = 0;

	virtual void Activate() { m_isActive = true; }
	virtual void Deactivate() { m_isActive = false; }
	virtual void Hide() { m_isVisible = false; }
	virtual void Show() { m_isVisible = true; }
	bool		 IsVisible() const { return m_isVisible; }
	bool		 IsActive() const { return m_isActive; }

	int GetId() const { return m_id; }
	std::string GetName() const { return m_name; }
	void SetBackgroundTexture( Texture* backgroundTexture ) { m_backgroundTexture = backgroundTexture; }
	void SetInitialTint( const Rgba8& tint ) { m_initialTint = m_curTint = tint; }
	void SetHoverTint( const Rgba8& tint ) { m_hoverTint = tint; }
	void ResetTint() { m_curTint = m_initialTint; }
	void ActivateHoverTint() { m_curTint = m_hoverTint; }
	void SetButtonAndLabelTint( const Rgba8& tint );
	AABB2 GetBoundingBox() const { return m_boundingBox; }
	Vec2 GetBoundingBoxCenter() const;
	void SetBoundingBoxPercent( const Vec2& percentOfDimensions );

	NamedProperties* GetUserData() const { return m_userData; }
	void SetUserData( NamedProperties* userData );

	// Virtual methods for adding to panels
	virtual UIPanel* AddChildPanel( const UIAlignedPositionData& positionData,
									Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
									const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
	virtual UIPanel* AddChildPanel( const UIRelativePositionData& positionData,
									Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
									const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
	virtual UIPanel* AddChildPanel( const std::string& name,
									const UIAlignedPositionData& positionData,
									Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
									const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );
	virtual UIPanel* AddChildPanel( const std::string& name,
									const UIRelativePositionData& positionData,
									Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE,
									const Vec2& uvAtMins = Vec2::ZERO, const Vec2& uvAtMaxs = Vec2::ONE );

	virtual UIButton* AddButton( const UIAlignedPositionData& positionData,
								 Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
	virtual UIButton* AddButton( const UIRelativePositionData& positionData,
								Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
	virtual UIButton* AddButton( const std::string& name,
								 const UIAlignedPositionData& positionData,
								 Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
	virtual UIButton* AddButton( const std::string& name,
								 const UIRelativePositionData& positionData,
								 Texture* backgroundTexture = nullptr, const Rgba8& tint = Rgba8::WHITE );
								 
	virtual UIUniformGrid* AddUniformGrid( const std::string& name, const UIAlignedPositionData& gridPositionData, 
										   const IntVec2& gridDimensions, const Vec2& paddingOfGridElementsPixels = Vec2::ZERO,
										   Texture* elementTexture = nullptr, const Rgba8& elementTint = Rgba8::WHITE );

	// Add methods for labels
	UILabel* AddImage( const UIAlignedPositionData& positionData,	Texture* image = nullptr );
	UILabel* AddImage( const UIRelativePositionData& positionData,	Texture* image = nullptr );
	UILabel* AddImage( const UIAlignedPositionData& positionData,	SpriteDefinition* spriteDef = nullptr );
	UILabel* AddImage( const UIRelativePositionData& positionData,	SpriteDefinition* spriteDef = nullptr );
	UILabel* AddImage( const std::string& name, const UIAlignedPositionData& positionData, Texture* image = nullptr );
	UILabel* AddImage( const std::string& name, const UIRelativePositionData& positionData, Texture* image = nullptr );
	UILabel* AddImage( const std::string& name, const UIAlignedPositionData& positionData, SpriteDefinition* spriteDef = nullptr );
	UILabel* AddImage( const std::string& name, const UIRelativePositionData& positionData, SpriteDefinition* spriteDef = nullptr );
	
	UILabel* AddText ( const UIAlignedPositionData& positionData,	const std::string& text, float fontSize = 24.f, const Vec2& textAlignment = ALIGN_CENTERED );
	UILabel* AddText ( const UIRelativePositionData& positionData,	const std::string& text, float fontSize = 24.f, const Vec2& textAlignment = ALIGN_CENTERED );
	UILabel* AddText ( const std::string& name, const UIAlignedPositionData& positionData,	const std::string& text, float fontSize = 24.f, const Vec2& textAlignment = ALIGN_CENTERED );
	UILabel* AddText ( const std::string& name, const UIRelativePositionData& positionData,	const std::string& text, float fontSize = 24.f, const Vec2& textAlignment = ALIGN_CENTERED );

	void	 ClearLabels();

	// Static methods
	static uint GetNextId();

protected:
	UIElement( UISystem& uiSystem, const std::string& name = "" );

protected:
	UISystem& m_uiSystem;

	int m_id = -1;
	std::string m_name;
	NamedProperties* m_userData = nullptr;
	bool m_isActive = true;
	bool m_isVisible = true;

	AABB2 m_initialBoundingBox;
	AABB2 m_boundingBox;
	Rgba8 m_initialTint = Rgba8::WHITE;
	Rgba8 m_curTint = Rgba8::WHITE;
	Rgba8 m_hoverTint = Rgba8::WHITE;
	Vec2 m_uvsAtMins = Vec2::ZERO;
	Vec2 m_uvsAtMaxs = Vec2::ONE;

	Texture* m_backgroundTexture = nullptr;

	std::vector<UILabel*> m_labels;

private:
	static int s_nextId;
};

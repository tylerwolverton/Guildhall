#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
class RenderContext;
class Texture;


//-----------------------------------------------------------------------------------------------
class UIElement
{
public:
	UIElement() = default;

	virtual void Update() = 0;
	virtual void Render( RenderContext* renderer ) const = 0;

	virtual void Activate()													{ m_isActive = true; }
	virtual void Deactivate()												{ m_isActive = false; }
	virtual void Hide()														{ m_isVisible = false; }
	virtual void Show()														{ m_isVisible = true; }

	uint GetId() const														{ return m_id; }
	void SetBackgroundTexture( Texture* backgroundTexture )					{ m_backgroundTexture = backgroundTexture; }
	void SetTint( const Rgba8& tint )										{ m_tint = tint; }
	AABB2 GetBoundingBox() const											{ return m_boundingBox; }

	// Static methods
	static uint GetNextId();

protected:
	uint m_id = 0;
	bool m_isActive = true;
	bool m_isVisible = true;

	AABB2 m_boundingBox;
	Rgba8 m_tint = Rgba8::WHITE;

	Texture* m_backgroundTexture = nullptr;

private:
	static uint s_nextId;
};

#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB2.hpp"


//-----------------------------------------------------------------------------------------------
class Texture;
class RenderBuffer;


//-----------------------------------------------------------------------------------------------
enum eCameraClearBitFlag : unsigned int
{
	CLEAR_COLOR_BIT = ( 1 << 0 ),
	CLEAR_DEPTH_BIT = ( 1 << 1 ),
	CLEAR_STENCIL_BIT = ( 1 << 2 ),
};


//-----------------------------------------------------------------------------------------------
struct CameraData
{
	Mat44 view;
	Mat44 projection;
};


//-----------------------------------------------------------------------------------------------
class Camera
{
public:
	Camera();
	~Camera();

	void SetOrthoView( const Vec2& bottomLeft, const Vec2& topRight );
	void SetOrthoView( const AABB2& cameraBounds );

	const Vec3 GetPosition()																{ return m_position; }
	void SetPosition( const Vec3& position );
	void Translate( const Vec3& translation );
	void Translate2D( const Vec2& translation);

	Vec2 GetOrthoBottomLeft()			const	{ return m_bottomLeft; }
	Vec2 GetOrthoTopRight()				const	{ return m_topRight; }
	//const AABB2 GetOrthoBounds()		const	{ return AABB2( m_bottomLeft, m_topRight ); }
	Rgba8 GetClearColor()				const	{ return m_clearColor; }
	unsigned int GetClearMode() const			{ return m_clearMode; }

	void SetClearMode( unsigned int clearFlags, Rgba8 color, float depth = 0.f, unsigned int stencil = 0 );

	void SetColorTarget( Texture* texture );
	Texture* GetColorTarget() const;

	// RednerContext* UpdateAndGetUBO(RenderContext* context);
	void UpdateCameraUBO();

public:
	RenderBuffer* m_cameraUBO = nullptr;

private:
	Vec2 m_bottomLeft;
	Vec2 m_topRight;
	
	Vec3 m_position;

	unsigned int m_clearMode = 0;
	Rgba8 m_clearColor = Rgba8::BLACK;

	Texture* m_colorTarget = nullptr;

	Mat44 m_projectionMatrix;
	Mat44 m_viewMatrix;
};


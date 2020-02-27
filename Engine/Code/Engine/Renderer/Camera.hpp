#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Transform.hpp"


//-----------------------------------------------------------------------------------------------
class Texture;
class RenderBuffer;


//-----------------------------------------------------------------------------------------------
enum eCameraClearBitFlag : uint
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
	// TODO: Add RenderContext as friend to improve encapsulation?
	//friend class RenderContext;

public:
	Camera();
	~Camera();
	
	// Transform
	void SetPosition( const Vec3& position );
	void Translate( const Vec3& translation );
	void SetPitchRollYawRotation( float pitch, float roll, float yaw );

	void SetProjectionOrthographic( float height, float nearZ = 1.f, float farZ = -1.f );
	void SetProjectionPerspective( float fovDegrees, float nearZClip, float farZClip );

	// Rendering
	void SetClearMode( unsigned int clearFlags, Rgba8 color, float depth = 0.f, unsigned int stencil = 0 );
	
	void SetColorTarget( Texture* texture );
	void SetDepthStencilTarget( Texture* texture );
	
	Vec3 ClientToWorldPosition( const Vec2& clientPos, float ndcZ = 0.f ) const;

	// RenderContext* UpdateAndGetUBO(RenderContext* context);
	void UpdateCameraUBO();

	// Accessors
	const Transform GetTransform()		const	{ return m_transform; }

	Vec2 GetOrthoMin() const;
	Vec2 GetOrthoMax() const;
	Rgba8 GetClearColor()				const	{ return m_clearColor; }
	unsigned int GetClearMode()			const	{ return m_clearMode; }
	Texture* GetColorTarget()			const;

	const Mat44 GetViewMatrix()			const	{ return m_viewMatrix; }
	const Mat44 GetProjectionMatrix()	const	{ return m_projectionMatrix; }

	// Helpers
	// can use this to determine aspect ratio
	void SetOutputSize( const Vec2& size );
	float GetAspectRatio() const;

public:
	RenderBuffer* m_cameraUBO = nullptr;

private:
	Vec2			m_bottomLeft;
	Vec2			m_topRight;
	Vec2			m_outputSize;
	
	Transform		m_transform;

	unsigned int	m_clearMode = 0;
	Rgba8			m_clearColor = Rgba8::BLACK;

	Texture*		m_colorTarget = nullptr;
	Texture*		m_deptStencilTarget = nullptr;

	Vec3 m_position;
	Mat44			m_projectionMatrix;
	Mat44			m_viewMatrix;
};

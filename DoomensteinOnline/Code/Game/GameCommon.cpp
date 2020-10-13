#include "Game/GameCommon.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"


//-----------------------------------------------------------------------------------------------
App*				g_app = nullptr;				// Owned by Main_Windows.cpp
Window*				g_window = nullptr;				// Owned by Main_Windows.cpp
RenderContext*		g_renderer = nullptr;			// Owned by the App
NetworkingSystem*	g_networkingSystem = nullptr;	// Owned by the App
InputSystem*		g_inputSystem = nullptr;		// Owned by the App
AudioSystem*		g_audioSystem = nullptr;		// Owned by the App
Game*				g_game = nullptr;				// Owned by the App
Server*				g_server = nullptr;				// Owned by the App
Client*				g_playerClient = nullptr;		// Owned by the App


//-----------------------------------------------------------------------------------------------
eBillboardStyle GetBillboardStyleFromString( const std::string& billboardStyleStr )
{
	if ( billboardStyleStr == "CameraFacingXY" )	{ return eBillboardStyle::CAMERA_FACING_XY; }
	if ( billboardStyleStr == "CameraOpposingXY" )	{ return eBillboardStyle::CAMERA_OPPOSING_XY; }
	if ( billboardStyleStr == "CameraFacingXYZ" )	{ return eBillboardStyle::CAMERA_FACING_XYZ; }
	if ( billboardStyleStr == "CameraOpposingXYZ" ) { return eBillboardStyle::CAMERA_OPPOSING_XYZ; }

	return eBillboardStyle::CAMERA_FACING_INVALID;
}


//-----------------------------------------------------------------------------------------------
void BillboardSpriteCameraFacingXY( const Vec2& pos, const Vec2& dimensions, const Camera& camera, Vec3* out_fourCorners )
{
	Vec3 forward = Vec3( camera.GetTransform().GetPosition().XY() - pos, 0.f ).GetNormalized();
	Vec3 left( forward.XY().GetRotated90Degrees(), 0.f );
	Vec3 up( 0.f, 0.f, 1.f );

	Vec3 bottomLeft( Vec3( pos, 0.f ) - ( left * dimensions.x * .5f ) );
	Vec3 bottomRight( Vec3( pos, 0.f ) + ( left * dimensions.x * .5f ) );
	Vec3 topLeft( bottomLeft + ( up * dimensions.y ) );
	Vec3 topRight( bottomRight + ( up * dimensions.y ) );

	out_fourCorners[0] = bottomLeft;
	out_fourCorners[1] = bottomRight;
	out_fourCorners[2] = topLeft;
	out_fourCorners[3] = topRight;
}


//-----------------------------------------------------------------------------------------------
void BillboardSpriteCameraOpposingXY( const Vec2& pos, const Vec2& dimensions, const Camera& camera, Vec3* out_fourCorners )
{
	Vec3 forward = Vec3( -camera.GetTransform().GetForwardVector().XY(), 0.f ).GetNormalized();
	Vec3 left( forward.XY().GetRotated90Degrees(), 0.f );
	Vec3 up( 0.f, 0.f, 1.f );

	Vec3 bottomLeft( Vec3( pos, 0.f ) - ( left * dimensions.x * .5f ) );
	Vec3 bottomRight( Vec3( pos, 0.f ) + ( left * dimensions.x * .5f ) );
	Vec3 topLeft( bottomLeft + ( up * dimensions.y ) );
	Vec3 topRight( bottomRight + ( up * dimensions.y ) );

	out_fourCorners[0] = bottomLeft;
	out_fourCorners[1] = bottomRight;
	out_fourCorners[2] = topLeft;
	out_fourCorners[3] = topRight;
}


//-----------------------------------------------------------------------------------------------
void BillboardSpriteCameraFacingXYZ( const Vec2& pos, const Vec2& dimensions, const Camera& camera, Vec3* out_fourCorners )
{
	Vec3 forward = Vec3( camera.GetTransform().GetPosition() - Vec3( pos, dimensions.y * .5f ) ).GetNormalized();
	Vec3 left = CrossProduct3D( Vec3( 0.f, 0.f, 1.f ), forward ).GetNormalized();
	Vec3 up = CrossProduct3D( forward, left );

	Vec3 bottomLeft( Vec3( pos, 0.f ) - ( left * dimensions.x * .5f ) );
	Vec3 bottomRight( Vec3( pos, 0.f ) + ( left * dimensions.x * .5f ) );
	Vec3 topLeft( bottomLeft + ( up * dimensions.y ) );
	Vec3 topRight( bottomRight + ( up * dimensions.y ) );

	out_fourCorners[0] = bottomLeft;
	out_fourCorners[1] = bottomRight;
	out_fourCorners[2] = topLeft;
	out_fourCorners[3] = topRight;
}


//-----------------------------------------------------------------------------------------------
void BillboardSpriteCameraOpposingXYZ( const Vec2& pos, const Vec2& dimensions, const Camera& camera, Vec3* out_fourCorners )
{
	Vec3 forward = Vec3( -camera.GetTransform().GetForwardVector() ).GetNormalized();
	// negative left vector to maintain right handed coordinate system after flipping forward vector
	Vec3 left = Vec3( camera.GetTransform().GetRightVector() ).GetNormalized(); 
	Vec3 up = Vec3( camera.GetTransform().GetUpVector() ).GetNormalized();

	Vec3 bottomLeft( Vec3( pos, 0.f ) - ( left * dimensions.x * .5f ) );
	Vec3 bottomRight( Vec3( pos, 0.f ) + ( left * dimensions.x * .5f ) );
	Vec3 topLeft( bottomLeft + ( up * dimensions.y ) );
	Vec3 topRight( bottomRight + ( up * dimensions.y ) );

	out_fourCorners[0] = bottomLeft;
	out_fourCorners[1] = bottomRight;
	out_fourCorners[2] = topLeft;
	out_fourCorners[3] = topRight;
}

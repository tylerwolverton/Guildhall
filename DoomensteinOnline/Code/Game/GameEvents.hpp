#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"


//-----------------------------------------------------------------------------------------------
class Entity;


//-----------------------------------------------------------------------------------------------
enum class eClientFunctionType
{
	NONE,
	MOVE_PLAYER,
	SET_PLAYER_ORIENTATION,
	POSSESS_ENTITY,
	UNPOSSESS_ENTITY,
};


//-----------------------------------------------------------------------------------------------
// TODO: This will become a message over the network later
struct ClientRequest
{
public:
	Entity*& player;
	eClientFunctionType functionType = eClientFunctionType::NONE;

public:
	ClientRequest( Entity*& playerIn, eClientFunctionType functionTypeIn )
		: player( playerIn )
		, functionType( functionTypeIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct PossessEntityRequest : ClientRequest
{
public:
	Transform cameraTransform;

public:
	PossessEntityRequest( Entity*& playerIn, const Transform& cameraTransformIn )
		: ClientRequest( playerIn, eClientFunctionType::POSSESS_ENTITY )
		, cameraTransform( cameraTransformIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct UnPossessEntityRequest : ClientRequest
{

public:
	UnPossessEntityRequest( Entity*& playerIn )
		: ClientRequest( playerIn, eClientFunctionType::UNPOSSESS_ENTITY )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct MovePlayerRequest : ClientRequest
{
public:
	Vec2 translationVec = Vec2::ZERO;

public:
	MovePlayerRequest( Entity*& playerIn, const Vec2& translationVecIn )
		: ClientRequest( playerIn, eClientFunctionType::MOVE_PLAYER )
		, translationVec( translationVecIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct SetPlayerOrientationRequest : ClientRequest
{
public:
	float yawOrientationDegrees = 0.f;

public:
	SetPlayerOrientationRequest( Entity*& playerIn, float yawOrientationDegreesIn )
		: ClientRequest( playerIn, eClientFunctionType::SET_PLAYER_ORIENTATION )
		, yawOrientationDegrees( yawOrientationDegreesIn )
	{
	}
};

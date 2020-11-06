#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
class Entity;


//-----------------------------------------------------------------------------------------------
enum class eClientFunctionType
{
	// CreateEntity
	// UpdateEntity
	// PlayerInput

	NONE,
	CREATE_ENTITY,
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
	EntityId playerClientId = -1;
	eClientFunctionType functionType = eClientFunctionType::NONE;

public:
	ClientRequest( Entity*& playerIn, eClientFunctionType functionTypeIn )
		: player( playerIn )
		, functionType( functionTypeIn )
	{
	}

	ClientRequest( Entity*& playerIn, EntityId playerId, eClientFunctionType functionTypeIn )
		: player( playerIn )
		, playerClientId( playerId )
		, functionType( functionTypeIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct CreateEntityRequest : ClientRequest
{
public:
	std::string entityType;
	Vec2 position = Vec2::ZERO;
	float yawOrientationDegrees = 0.f;

public:
	CreateEntityRequest( Entity*& playerIn, const std::string& entityTypeIn, const Vec2& positionIn, float yawOrientationDegreesIn )
		: ClientRequest( playerIn, eClientFunctionType::CREATE_ENTITY )
		, entityType( entityTypeIn )
		, position( positionIn )
		, yawOrientationDegrees( yawOrientationDegreesIn )
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


// TODO: Combine movement and orientation into one entity update struct
//-----------------------------------------------------------------------------------------------
struct MovePlayerRequest : ClientRequest
{
public:
	Vec2 translationVec = Vec2::ZERO;

public:
	MovePlayerRequest( Entity*& playerIn, EntityId playerId, const Vec2& translationVecIn )
		: ClientRequest( playerIn, playerId, eClientFunctionType::MOVE_PLAYER )
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

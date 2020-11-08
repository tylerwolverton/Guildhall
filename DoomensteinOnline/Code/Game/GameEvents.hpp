#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Game/GameCommon.hpp"
#include "Game/EntityDefinition.hpp"


//-----------------------------------------------------------------------------------------------
class Entity;


//-----------------------------------------------------------------------------------------------
enum eClientFunctionType
{
	// CreateEntity
	// UpdateEntity
	// PlayerInput

	INVALID = -1,

	CREATE_ENTITY,
	UPDATE_ENTITY,
	POSSESS_ENTITY,
	UNPOSSESS_ENTITY,

	NUM_TYPES
};


//-----------------------------------------------------------------------------------------------
// TODO: This will become a message over the network later
struct ClientRequest
{
public:
	int clientId = -1;
	EntityId entityId = -1;
	eClientFunctionType functionType = eClientFunctionType::INVALID;

public:
	ClientRequest( int clientIdIn, EntityId entityIdIn, eClientFunctionType functionTypeIn )
		: clientId( clientIdIn )
		, entityId( entityIdIn )
		, functionType( functionTypeIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct CreateEntityRequest : ClientRequest
{
public:
	eEntityType entityType;
	float yawOrientationDegrees = 0.f;
	Vec2 position = Vec2::ZERO;

public:
	CreateEntityRequest( int clientIdIn, EntityId entityIdIn, eEntityType entityTypeIn, const Vec2& positionIn, float yawOrientationDegreesIn )
		: ClientRequest( clientIdIn, entityIdIn, eClientFunctionType::CREATE_ENTITY )
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
	PossessEntityRequest( int clientIdIn, EntityId entityIdIn, const Transform& cameraTransformIn )
		: ClientRequest( clientIdIn, entityIdIn, eClientFunctionType::POSSESS_ENTITY )
		, cameraTransform( cameraTransformIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct UnPossessEntityRequest : ClientRequest
{

public:
	UnPossessEntityRequest( int clientIdIn, EntityId entityIdIn )
		: ClientRequest( clientIdIn, entityIdIn, eClientFunctionType::UNPOSSESS_ENTITY )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct UpdateEntityRequest : ClientRequest
{
public:
	float yawOrientationDegrees = 0.f;
	Vec2 translationVec = Vec2::ZERO;

public:
	UpdateEntityRequest( int clientIdIn, EntityId entityIdIn, const Vec2& translationVecIn, float yawOrientationDegreesIn )
		: ClientRequest( clientIdIn, entityIdIn, eClientFunctionType::UPDATE_ENTITY )
		, translationVec( translationVecIn )
		, yawOrientationDegrees( yawOrientationDegreesIn )
	{
	}
};

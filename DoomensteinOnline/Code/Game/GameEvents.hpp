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

	REQUEST_CONNECTION,
	RESPONSE_TO_CONNECTION_REQUEST,

	NUM_TYPES
};


//-----------------------------------------------------------------------------------------------
struct ClientRequest
{
public:
	int key = -1;
	int clientId = -1;
	eClientFunctionType functionType = eClientFunctionType::INVALID;

public:
	ClientRequest( int clientIdIn, eClientFunctionType functionTypeIn )
		: clientId( clientIdIn )
		, functionType( functionTypeIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct RequestConnectionRequest : ClientRequest
{
public:

public:
	RequestConnectionRequest( int clientIdIn )
		: ClientRequest( clientIdIn, eClientFunctionType::REQUEST_CONNECTION )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct ResponseToConnectionRequest : ClientRequest
{
public:
	int connectKey = -1;
	int port = - 1;
	uint16_t size = 0;

public:
	ResponseToConnectionRequest( int clientIdIn, int keyIn, int portIn, uint16_t sizeIn )
		: ClientRequest( clientIdIn, eClientFunctionType::RESPONSE_TO_CONNECTION_REQUEST )
		, connectKey( keyIn )
		, port( portIn )
		, size( sizeIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct CreateEntityRequest : ClientRequest
{
public:
	EntityId entityId = -1;
	eEntityType entityType;
	float yawOrientationDegrees = 0.f;
	Vec2 position = Vec2::ZERO;

public:
	CreateEntityRequest( int clientIdIn, EntityId entityIdIn, eEntityType entityTypeIn, const Vec2& positionIn, float yawOrientationDegreesIn )
		: ClientRequest( clientIdIn, eClientFunctionType::CREATE_ENTITY )
		, entityId( entityIdIn )
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
	EntityId entityId = -1;
	Transform cameraTransform;

public:
	PossessEntityRequest( int clientIdIn, EntityId entityIdIn, const Transform& cameraTransformIn )
		: ClientRequest( clientIdIn, eClientFunctionType::POSSESS_ENTITY )
		, entityId( entityIdIn )
		, cameraTransform( cameraTransformIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct UnPossessEntityRequest : ClientRequest
{
public:
	EntityId entityId = -1;

public:
	UnPossessEntityRequest( int clientIdIn, EntityId entityIdIn )
		: ClientRequest( clientIdIn, eClientFunctionType::UNPOSSESS_ENTITY )
		, entityId( entityIdIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct UpdateEntityRequest : ClientRequest
{
public:
	EntityId entityId = -1;
	float yawOrientationDegrees = 0.f;
	Vec2 translationVec = Vec2::ZERO;

public:
	UpdateEntityRequest( int clientIdIn, EntityId entityIdIn, const Vec2& translationVecIn, float yawOrientationDegreesIn )
		: ClientRequest( clientIdIn, eClientFunctionType::UPDATE_ENTITY )
		, entityId( entityIdIn )
		, translationVec( translationVecIn )
		, yawOrientationDegrees( yawOrientationDegreesIn )
	{
	}
};

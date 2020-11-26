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
	UPDATE_ENTITY_ON_REMOTE_SERVER,
	POSSESS_ENTITY,
	UNPOSSESS_ENTITY,
	SET_PLAYER_ID,
	SET_PLAYER_ID_ACK,

	REQUEST_CONNECTION,
	RESPONSE_TO_CONNECTION_REQUEST,
	KEY_VERIFICATION,
	REMOTE_CLIENT_REGISTRATION,

	SERVER_LAST_DELTA_SECONDS,

	NUM_TYPES
};


//-----------------------------------------------------------------------------------------------
// TODO: Virtualize destructors
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
	int distantBindPort = - 1;
	int localSendToPort = - 1;
	uint16_t size = 0;

public:
	ResponseToConnectionRequest( int clientIdIn, int keyIn, int portIn, int listenPortIn, uint16_t sizeIn )
		: ClientRequest( clientIdIn, eClientFunctionType::RESPONSE_TO_CONNECTION_REQUEST )
		, connectKey( keyIn )
		, distantBindPort( portIn )
		, localSendToPort( listenPortIn )
		, size( sizeIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct KeyVerificationRequest : ClientRequest
{
public:
	int connectKey = -1;

public:
	KeyVerificationRequest( int clientIdIn, int keyIn )
		: ClientRequest( clientIdIn, eClientFunctionType::KEY_VERIFICATION )
		, connectKey( keyIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct RemoteClientRegistrationRequest : ClientRequest
{
public:

public:
	RemoteClientRegistrationRequest( int clientIdIn )
		: ClientRequest( clientIdIn, eClientFunctionType::REMOTE_CLIENT_REGISTRATION )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct ServerLastDeltaSecondsRequest : ClientRequest
{
public:
	float deltaSeconds = .01667f;

public:
	ServerLastDeltaSecondsRequest( int clientIdIn, float deltaSecondsIn )
		: ClientRequest( clientIdIn, eClientFunctionType::SERVER_LAST_DELTA_SECONDS )
		, deltaSeconds( deltaSecondsIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct CreateEntityRequest : ClientRequest
{
public:
	EntityId entityId = -1;
	float yawOrientationDegrees = 0.f;
	Vec2 position = Vec2::ZERO;
	eEntityType entityType = eEntityType::NONE;

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
	float yawRotationDegrees = 0.f;
	Vec2 translationVec = Vec2::ZERO;

public:
	UpdateEntityRequest( int clientIdIn, EntityId entityIdIn, const Vec2& translationVecIn, float yawRotationDegreesIn )
		: ClientRequest( clientIdIn, eClientFunctionType::UPDATE_ENTITY )
		, entityId( entityIdIn )
		, translationVec( translationVecIn )
		, yawRotationDegrees( yawRotationDegreesIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct UpdateEntityOnRemoteServerRequest : ClientRequest
{
public:
	EntityId entityId = -1;
	float yawOrientationDegrees = 0.f;
	Vec2 positionVec = Vec2::ZERO;

public:
	UpdateEntityOnRemoteServerRequest( int clientIdIn, EntityId entityIdIn, const Vec2& positionVecIn, float yawOrientationDegreesIn )
		: ClientRequest( clientIdIn, eClientFunctionType::UPDATE_ENTITY_ON_REMOTE_SERVER )
		, entityId( entityIdIn )
		, positionVec( positionVecIn )
		, yawOrientationDegrees( yawOrientationDegreesIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct SetPlayerIdRequest : ClientRequest
{
public:
	EntityId playerId = -1;

public:
	SetPlayerIdRequest( int clientIdIn, EntityId playerIdIn )
		: ClientRequest( clientIdIn, eClientFunctionType::SET_PLAYER_ID )
		, playerId( playerIdIn )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct SetPlayerIdAckRequest : ClientRequest
{
public:

public:
	SetPlayerIdAckRequest( int clientIdIn )
		: ClientRequest( clientIdIn, eClientFunctionType::SET_PLAYER_ID_ACK )
	{
	}
};

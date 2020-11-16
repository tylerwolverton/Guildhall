#pragma once


//-----------------------------------------------------------------------------------------------
struct ClientRequest;
class Entity;
class World;


//-----------------------------------------------------------------------------------------------
class Client
{
public:
	Client() = default;
	~Client() = default;

	virtual void Startup();
	virtual void Shutdown();

	virtual void BeginFrame();

	virtual void Update();
	virtual void SendUpdatedGameWorldToServer();
	virtual void Render( const World* gameWorld ) const;

	virtual int  GetClientId()												{ return m_clientId; }
	virtual void SetClientId( int id ) = 0; 
	virtual void SetPlayer( Entity* entity ) = 0;

	virtual void SendMessageToDistantClient( ClientRequest* message ) = 0;

protected:
	int m_clientId = -1;
};

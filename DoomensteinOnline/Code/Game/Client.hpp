#pragma once


//-----------------------------------------------------------------------------------------------
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
	virtual void Render( const World* gameWorld ) const;

	virtual void SetClientId( int id )							{ m_clientId = id; }
	virtual void SetPlayer( Entity* entity );

protected:
	int m_clientId = -1;
};

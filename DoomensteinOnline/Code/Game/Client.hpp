#pragma once


//-----------------------------------------------------------------------------------------------
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
};

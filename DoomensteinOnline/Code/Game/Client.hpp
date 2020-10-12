#pragma once
//-----------------------------------------------------------------------------------------------
class PlayerClient;
class RemoteClient;


//-----------------------------------------------------------------------------------------------
class Client
{
public:
	Client() = default;
	~Client() = default;

	virtual void Startup();
	virtual void Shutdown();

	void Render() const;

private:
	PlayerClient* m_playerClient = nullptr;
	RemoteClient* m_remoteClient = nullptr;
};

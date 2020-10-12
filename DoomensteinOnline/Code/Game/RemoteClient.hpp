#pragma once
class RemoteClient
{
public:
	RemoteClient() = default;
	~RemoteClient() = default;

	virtual void Startup();
	virtual void Shutdown();


private:

};

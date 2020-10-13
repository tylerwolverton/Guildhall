#pragma once


//-----------------------------------------------------------------------------------------------
class Client
{
public:
	Client() = default;
	~Client() = default;

	virtual void Startup();
	virtual void Shutdown();

	virtual void Render() const;
};

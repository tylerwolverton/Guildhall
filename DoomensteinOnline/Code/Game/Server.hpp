#pragma once
//-----------------------------------------------------------------------------------------------
class Server
{
public:
	Server() = default;
	virtual ~Server() = default;

	virtual void Startup() = 0;
	virtual void Shutdown() = 0;

	virtual void Update() = 0;
};

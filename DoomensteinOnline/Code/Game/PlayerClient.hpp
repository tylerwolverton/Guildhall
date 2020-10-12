#pragma once
class PlayerClient
{
public:
	PlayerClient() = default;
	~PlayerClient() = default;

	virtual void Startup();
	virtual void Shutdown();

	void Render() const;

private:

};

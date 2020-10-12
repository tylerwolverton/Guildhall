#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class Camera;
class RenderContext;
class Game;
enum class eWindowMode;


//-----------------------------------------------------------------------------------------------
enum class eAppMode
{
	SINGLE_PLAYER,
	MULTIPLAYER_SERVER,
	MULTIPLAYER_CLIENT,
	HEADLESS_SERVER,
};


//-----------------------------------------------------------------------------------------------
class App 
{
public:
	App();
	~App();
	void Startup( eAppMode appMode = eAppMode::SINGLE_PLAYER );
	void Shutdown();
	void RunFrame();

	bool IsQuitting()										{ return m_isQuitting; }
	
	bool HandleQuitRequested();
	void RestartGame();
	
private:
	void PopulateGameConfig();
	eWindowMode GetWindowModeFromGameConfig();
	void BeginFrame();
	void Update();
	void UpdateFromKeyboard();
	void Render() const;
	void EndFrame();

	static bool QuitGame( EventArgs* args );

private:
	bool m_isQuitting = false;
	eAppMode m_appMode = eAppMode::SINGLE_PLAYER;
};

#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
class Camera;
class RenderContext;
class Game;
enum class eWindowMode;


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
	
private:
	void PopulateGameConfig();
	eWindowMode GetWindowModeFromGameConfig();
	void BeginFrame();
	void Update();
	void UpdateFromKeyboard();
	void Render() const;
	void EndFrame();

	void RestartApp( eAppMode appMode = eAppMode::SINGLE_PLAYER, EventArgs* args = nullptr );
	void InitializeServerAndClient( eAppMode appMode = eAppMode::SINGLE_PLAYER, EventArgs* args = nullptr );
	void DeallocateServerAndClient( eAppMode appMode = eAppMode::SINGLE_PLAYER );

	// Events
	static bool QuitGame( EventArgs* args );
	static bool StartMultiplayerServerCommand( EventArgs* args );
	static bool ConnectToMultiplayerServerCommand( EventArgs* args );

private:
	bool m_isQuitting = false;
	eAppMode m_appMode = eAppMode::SINGLE_PLAYER;
};

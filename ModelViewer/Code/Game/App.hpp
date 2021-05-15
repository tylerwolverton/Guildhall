#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"


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
	void Startup();
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

	bool CheckAndRunCommandlets();

	static bool QuitGame( EventArgs* args );

private:
	bool m_isQuitting = false;
};

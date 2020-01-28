#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class Camera;
class RenderContext;
class Game;
enum class WindowMode;


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
	WindowMode GetWindowModeFromGameConfig();
	void BeginFrame();
	void Update( float deltaSeconds );
	void UpdateFromKeyboard( float deltaSeconds );
	void Render() const;
	void EndFrame();
	
private:
	bool m_isQuitting = false;
};
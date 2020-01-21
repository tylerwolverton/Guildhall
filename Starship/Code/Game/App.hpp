#pragma once
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class Camera;
class RenderContext;
class Game;


//-----------------------------------------------------------------------------------------------
class App 
{
public:
	App();
	~App();
	void Startup();
	void Shutdown();
	void RunFrame();

	bool IsQuitting()									{ return m_isQuitting; }
	bool HandleQuitRequested();
	void RestartGame();
	
private:
	void BeginFrame();
	void Update( float deltaSeconds );
	void UpdateFromKeyboard( float deltaSeconds );
	void Render() const;
	void EndFrame();
	
private:
	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	bool m_isDebugRendering = false;

	Game* m_theGame = nullptr;

	float m_screenShakeIntensity = 0.f;
};
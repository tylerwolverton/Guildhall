#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/EngineCommon.hpp"


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

	bool IsQuitting() { return m_isQuitting; }

	bool HandleQuitRequested();
	void RestartGame();

	static bool QuitGame( EventArgs* args );

private:
	eWindowMode GetWindowModeFromGameConfig();

	void BeginFrame();
	void Update( float deltaSeconds );
	void UpdateFromKeyboard( float deltaSeconds );
	void Render() const;
	void EndFrame();

private:
	bool m_isQuitting = false;
};

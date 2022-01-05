#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
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
	void RestartEditor();

private:
	eWindowMode GetWindowModeFromGameConfig();

	void BeginFrame();
	void Update();
	void UpdateFromKeyboard();
	void Render() const;
	void EndFrame();

	static bool QuitGame( EventArgs* args );

private:
	bool m_isQuitting = false;
};

#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"

#include "Editor/EditorCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
struct Rgba8;
class RandomNumberGenerator;
class Clock;
class Camera;


//-----------------------------------------------------------------------------------------------
enum class eEditorState
{
	LOADING,
	PLAYING,
	PAUSED
};


//-----------------------------------------------------------------------------------------------
class Editor
{
public:
	Editor();
	~Editor();

	void		 Startup();
	void		 BeginFrame();
	void		 Update();
	void		 Render() const;
	void		 DebugRender() const;
	void		 EndFrame();
	void		 Shutdown();
				 
	void		 RestartEditor();
	
	void		 ChangeEditorState( const eEditorState& newGameState );
	eEditorState GetEditorState()															{ return m_editorState; }

	const Vec2	 GetMouseWorldPosition()													{ return m_mouseWorldPosition; }
				 
	void		 SetWorldCameraPosition( const Vec3& position );
				
	float		 GetLastDeltaSecondsf();
				 
public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void LoadAssets();
	void ReloadEditor();

	void UpdateFromKeyboard();
	void UpdateMousePositions();
	void UpdateMouseWorldPosition();
	void UpdateMouseUIPosition();
	void UpdateCameras();

	void InitializeFPSHistory();
	void UpdateFramesPerSecond();
	float GetAverageFPS() const;
	void RenderFPSCounter() const;

private:
	Clock* m_editorClock = nullptr;
	float m_fpsHistory[FRAME_HISTORY_COUNT];
	int m_fpsNextIdx = 0;
	float m_fpsHistorySum = 0.f;

	bool m_isPaused = false;
	bool m_isDebugRendering = false;

	Vec2 m_mouseWorldPosition = Vec2::ZERO;
	Vec2 m_mouseUIPosition = Vec2::ZERO;

	int m_loadingFrameNum = 0;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;
	Vec3 m_focalPoint = Vec3::ZERO;

	eEditorState m_editorState = eEditorState::LOADING;
};

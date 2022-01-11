#pragma once
#include "Engine/Math/Vec2.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class EventSystem;
class InputSystem;


//-----------------------------------------------------------------------------------------------
enum class eWindowMode
{
	WINDOWED,
	BORDERLESS,
};


//-----------------------------------------------------------------------------------------------
class Window
{
public:
	Window();
	~Window();

	void SetEventSystem( EventSystem* eventSystem );
	void SetInputSystem( InputSystem* inputSystem );

	bool Open( const std::string& title, float clientAspect = 16.f / 9.f, float maxClientFractionOfDesktop = .9f, eWindowMode windowMode = eWindowMode::WINDOWED );
	void Close();
	void BeginFrame();
	void EndFrame();

	unsigned int	GetClientWidth() const			{ return m_clientWidth; }
	unsigned int	GetClientHeight() const			{ return m_clientHeight; }
	Vec2			GetDimensions()	const			{ return Vec2( (float)GetClientWidth(), (float)GetClientHeight() ); }

	void			ToggleWindowMode();

	EventSystem*	GetEventSystem() const			{ return m_eventSystem; }
	InputSystem*	GetInputSystem() const			{ return m_inputSystem; }

private:
	void PopulateDesktopWindowData();
	Vec2 GetClientWindowDimensions( float clientAspect, float maxClientFractionOfDesktop, eWindowMode windowMode );
	void GetAdjustedWindowEdges( float clientAspect, float maxClientFractionOfDesktop, eWindowMode windowMode,
								   long& windowLeft, long& windowRight, long& windowTop, long& windowBottom );

	void UpdateClientWindowDimensions( float clientAspect, float maxClientFractionOfDesktop, eWindowMode windowMode );

public:
	void* m_hwnd = nullptr;
	unsigned int m_clientWidth = 0;
	unsigned int m_clientHeight = 0;

private:
	EventSystem* m_eventSystem = nullptr;
	InputSystem* m_inputSystem = nullptr;

	eWindowMode m_curWindowMode = eWindowMode::WINDOWED;

	float m_desktopWidth = 500.f;
	float m_desktopHeight = 500.f;
	float m_desktopAspect = 1.f;
};

#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class EventSystem;
class InputSystem;


//-----------------------------------------------------------------------------------------------
enum class WindowMode
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

	bool Open( const std::string& title, float clientAspect = 16.f / 9.f, float maxClientFractionOfDesktop = .9f, WindowMode windowMode = WindowMode::WINDOWED );
	void Close();
	void BeginFrame();
	void EndFrame();

	unsigned int GetClientWidth()				{ return m_clientWidth; }
	unsigned int GetClientHeight()				{ return m_clientHeight; }

	EventSystem* GetEventSystem() const			{ return m_eventSystem; }
	InputSystem* GetInputSystem() const			{ return m_inputSystem; }

public:
	void* m_hwnd = nullptr;
	unsigned int m_clientWidth = 0;
	unsigned int m_clientHeight = 0;

private:
	EventSystem* m_eventSystem = nullptr;
	InputSystem* m_inputSystem = nullptr;
};

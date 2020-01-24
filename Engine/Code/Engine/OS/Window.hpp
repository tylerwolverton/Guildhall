//#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class Window
{
public:
	Window();
	~Window();

	bool Open( const std::string& title, float clientAspect = 16.f / 9.f, float maxClientFractionOfDesktop = .9f, bool isBorderless = false );
	void Close();
	void BeginFrame();
	void EndFrame();

	unsigned int GetClientWidth() { return m_clientWidth; }
	unsigned int GetClientHeight() { return m_clientHeight; }

public:
	void* m_hwnd = nullptr;
	unsigned int m_clientWidth = 0;
	unsigned int m_clientHeight = 0;
};

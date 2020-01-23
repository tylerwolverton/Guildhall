//#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class Window
{
public:
	Window();
	~Window();

	bool Open( const std::string& title, float clientAspect = 16.f / 9.f, float maxClientFractionOfDesktop = .9f );
	void Close();
	void BeginFrame();

	unsigned int GetClientWidth() { return (unsigned int)m_clientWidth; }
	unsigned int GetClientHeight() { return (unsigned int)m_clientHeight; }

public:
	void* m_hwnd;
	float m_clientWidth;
	float m_clientHeight;
};

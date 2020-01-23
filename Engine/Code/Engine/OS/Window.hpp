//#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class Window
{
public:
	Window();
	~Window();

	bool Open( const std::string& title, float clientAspect = 16.f/9.f, float maxClientFractionOfDesktop = .9f );
	void Close();
	void BeginFrame();

public:
	void* m_hwnd;
};

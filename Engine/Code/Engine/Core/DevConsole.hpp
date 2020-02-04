#pragma once
#include "Engine/Core/Rgba8.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
class RenderContext;
class Camera;


//-----------------------------------------------------------------------------------------------
struct DevConsoleLogMessage
{
public:
	Rgba8 m_color = Rgba8::WHITE;
	std::string m_message;

public:
	explicit DevConsoleLogMessage( const Rgba8& color, std::string message )
		: m_color( color )
		, m_message( message )
	{}
};


//-----------------------------------------------------------------------------------------------
class DevConsole
{
public:
	DevConsole();
	~DevConsole();
	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void PrintString( const Rgba8& textColor, const std::string& devConsolePrintString );
	void Render( RenderContext& renderer, const Camera& camera, float lineHeight ) const;
	void Render( RenderContext& renderer, const AABB2& bounds, float lineHeight ) const;

	void ToggleOpenFull();
	void Close();
	bool IsOpen() const												{ return m_isOpen; }

private:
	void RenderBackground( RenderContext& renderer, const AABB2& bounds ) const;
	void RenderLatestLogMessages( RenderContext& renderer, const AABB2& bounds, float lineHeight ) const;

private:
	bool m_isOpen = false;
	Camera* m_devConsoleCamera = nullptr;
	std::vector<DevConsoleLogMessage> m_logMessages;
};

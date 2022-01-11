#pragma once

class RenderContext;
class Window;

class DearImguiSystem
{
public:
	static void Startup( const RenderContext& renderer, const Window& window );
	static void Shutdown();
	static void BeginFrame();
	static void Render();
};

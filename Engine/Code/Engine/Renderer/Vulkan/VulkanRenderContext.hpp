#pragma once
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
class Window;
class Clock;


//-----------------------------------------------------------------------------------------------
class VulkanRenderContext : public RenderContext
{
public:
	virtual void Startup( Window* window ) override;
	virtual void BeginFrame() override;
	virtual void EndFrame() override;
	virtual void Shutdown() override;
};

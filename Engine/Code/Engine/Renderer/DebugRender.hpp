#pragma once
class Camera;
class Texture;


/************************************************************************/
/*                                                                      */
/* TYPES                                                                */
/*                                                                      */
/************************************************************************/

// Only effects "world" mode stuff; 
enum eDebugRenderMode
{
	DEBUG_RENDER_ALWAYS,          // what is rendered always shows up
	DEBUG_RENDER_USE_DEPTH,       // respect the depth buffer
	DEBUG_RENDER_XRAY,            // renders twice - once darker when it shoudl be hidden, and once more saturated when it should appear
};

/************************************************************************/
/*                                                                      */
/* FUNCTION PROTOTYPES                                                  */
/*                                                                      */
/************************************************************************/
//------------------------------------------------------------------------
//  System
//------------------------------------------------------------------------
// setup
void DebugRenderSystemStartup();
void DebugRenderSystemShutdown();

// control
void EnableDebugRendering();
void DisableDebugRendering();

// output
void DebugRenderBeginFrame();                   // Does nothing, here for completeness.
void DebugRenderWorldToCamera( Camera* cam );   // Draws all world objects to this camera 
void DebugRenderScreenTo( Texture* output );    // Draws all screen objects onto this texture (screen coordinate system is up to you.  I like a 1080p default)
void DebugRenderEndFrame();                     // Clean up dead objects

//------------------------------------------------------------------------
// World Rendering
//------------------------------------------------------------------------
// points
void DebugAddWorldPoint( Vec3 pos, float size, const Rgba8& start_color, const Rgba8& end_color, float duration, eDebugRenderMode mode );
void DebugAddWorldPoint( Vec3 pos, float size, const Rgba8& color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );
void DebugAddWorldPoint( Vec3 pos, const Rgba8& color, float duration = 0.0f, eDebugRenderMode mode = DEBUG_RENDER_USE_DEPTH );


//------------------------------------------------------------------------
// Screen Rendering
//------------------------------------------------------------------------
void DebugRenderSetScreenHeight( float height ); // default to 1080.0f when system starts up.  Meaning (0,0) should always be bottom left, (aspect * height, height) is top right
AABB2 DebugGetScreenBounds();                    // useful if you want to align to top right for something

// points
void DebugAddScreenPoint( Vec2 pos, float size, const Rgba8& start_color, const Rgba8& end_color, float duration );
void DebugAddScreenPoint( Vec2 pos, float size, const Rgba8& color, float duration = 0.0f );
void DebugAddScreenPoint( Vec2 pos, const Rgba8& color ); // assumed size;
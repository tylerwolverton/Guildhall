Project: FPS

------

## Checklist [90%]
- [x] RenderContext
    - [x] `RasterState` moved off shader, and added to `RenderContext`
    - [x] `RenderContext::SetCullMode`
    - [x] `RenderContext::SetFillMode`
    - [x] `RenderContext::SetFrontFaceWindOrder`
    - [x] Create a default raster state to set when `BeginCamera` is called.
    - [x] Have a transient raster state to create/set when above calls are used.

- [ ] World Rendering
    - [x] Points
    - [-] Lines
    - [-] Arrows
    - [-] Basis
    - [x] Quad
    - [x] Wire Box
    - [x] Wire Sphere
    - [x] Text
    - [x] Billboarded Text
    - [x] All world commands support all rendering modes; 

- [x] Screen Rendering
    - [x] Points
    - [x] Lines
    - [x] Quads
    - [x] Textured Quads
    - [x] Text

- [x] Output
    - [x] Implement `DebugRenderWorldTo` to draw the debug objects into the passed camera's space.
    - [x] Implement `DebugRenderScreenTo` to add the screen-space debug calls to the passed in texture.
    - [x] Add a `DebugRenderWorldTo` call to your game after you render your scene
    - [x] Add a `DebugRenderScreenTo` call to your App before your present to render 2D objects

- [x] Controls
    - [x] Console command: `debug_render enabled=bool` 
    - [x] Console command: `debug_add_world_point position=vec3 duration=float`
    - [x] Console command: `debug_add_world_wire_sphere position=vec3 radius=float duration=float`
    - [x] Console command: `debug_add_world_wire_bounds min=vec3 max=vec3 duration=float`
    - [x] Console command: `debug_add_world_billboard_text position=vec3 pivot=vec2 text=string`
    - [x] Console command: `debug_add_screen_point position=vec2 duration=float`
    - [x] Console command: `debug_add_screen_quad min=vec2 max=vec2 duration=float`
    - [x] Console command: `debug_add_screen_text position=vec2 pivot=vec2 text=string`
        - Note: added duration to this one too  


## Extras
- [ ] *X05.00: 02%*: Screen Basis functions implemented
- [ ] *X05.00: 05%*: Message functions implemented
- [ ] *X05.00: 04%*: Border Around Text (one or two texel black border around text drawn in the world)
- [ ] *X05.00: 03%*: Option to include a background behind rendered text (color with alpha)
- [ ] *X05.00: 05%*: Option to pass bounds to text, and pass an alignment.  Text should wrap and align to within the box.  If the text would be larger than the box, you may handle this in your preferred way (clip overflow, center it, or shrink to fit)
- [ ] *X05.00: 02%*: `DebugAddWorldLineStrip`
- [ ] *X05.00: 02%*: `DebugAddWireMeshToWorld( mat44 modelMatrix, GPUMesh* mesh, rgba start_tint, rgba end_tint, float duration, eDebugRenderMode mode );` 
- [ ] *X05.00: 02%*: `DebugRenderAddGrid`, see notes.
    - [ ] *X05.00: 03%*: Grid is clipped to area the camera can potentially see (allowing for an infinitly large grid)
- [x] *X05.00: 03%*: MeshUtils: `AddCylinderToIndexedVertexArray`
- [x] *X05.00: 03%*: MeshUtils: `AddConeToIndexedVertexArray`
- [ ] *X05.00: 04%*: `RGBA LerpAsHSL( RGBA const &a, RGBA const &b, float t );` 

------


------
Controls

Dev Console
Tab - Autocomplete ( finish command or scroll through available commands when line is blank or command is present )
Shift Tab - Move backwards through Autocomplete suggestions

Game
F2 - Cycle Sampler (between point and bilinear)
F3 - Cycle Blend Mode (Affects the debug image in the top middle)
WASD ( C and Spacebar for up down ) - Move camera
Shift - Move faster 

------
Dev Console Commands
set_mouse_sensitivity multiplier=NUMBER - change multiplier for mouse sensitivity
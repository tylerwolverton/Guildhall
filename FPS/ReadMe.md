Project: FPS

------

## Checklist [90%]
- [ ] RenderContext
    - [x] `RasterState` moved off shader, and added to `RenderContext`
    - [ ] `RenderContext::SetCullMode`
    - [ ] `RenderContext::SetFillMode`
    - [ ] `RenderContext::SetFrontFaceWindOrder`
    - [ ] Create a default raster state to set when `BeginCamera` is called.
    - [ ] Have a transient raster state to create/set when above calls are used.

- [ ] World Rendering
    - [ ] Points
    - [ ] Lines
    - [ ] Arrows
    - [ ] Basis
    - [ ] Quad
    - [ ] Wire Box
    - [ ] Wire Sphere
    - [ ] Text
    - [ ] Billboarded Text
    - [ ] All world commands support all rendering modes; 

- [ ] Screen Rendering
    - [ ] Points
    - [ ] Lines
    - [ ] Quads
    - [ ] Textured Quads
    - [ ] Text

- [ ] Output
    - [ ] Implement `DebugRenderWorldTo` to draw the debug objects into the passed camera's space.
    - [ ] Implement `DebugRenderScreenTo` to add the screen-space debug calls to the passed in texture.
    - [ ] Add a `DebugRenderWorldTo` call to your game after you render your scene
    - [ ] Add a `DebugRenderScreenTo` call to your App before your present to render 2D objects

- [ ] Controls
    - [ ] Console command: `debug_render enabled=bool` 
    - [ ] Console command: `debug_add_world_point position=vec3 duration=float`
    - [ ] Console command: `debug_add_world_wire_disc position=vec3 radius=float duration=float`
    - [ ] Console command: `debug_add_world_wire_bounds min=vec3 max=vec3 duration=float`
    - [ ] Console command: `debug_add_world_billboard_text position=vec3 pivot=vec2 text=string`
    - [ ] Console command: `debug_add_screen_point position=vec2 duration=float`
    - [ ] Console command: `debug_add_screen_quad min=vec2 max=vec2 duration=float`
    - [ ] Console command: `debug_add_screen_text position=vec2 pivot=vec2 text=string`


## Extras
- [ ] Screen Basis functions implemented
- [ ] Message functions implemented
- [ ] Border Around Text (one or two texel black border around text drawn in the world)
- [ ] Option to include a background behind rendered text (color with alpha)
- [ ] Option to pass bounds to text, and pass an alignment.  Text should wrap and align to within the box.  If the text
      would be larger than the box, you may handle this in your preferred way (clip overflow, center it, or shrink to fit)
- [ ] `DebugAddWorldLineStrip`
- [ ] `DebugAddWireMeshToWorld( mat44 mode, GPUMesh* mesh, rgba start_tint, rgba end_tint, float duration, eDebugRenderMode mode );` 
- [ ] `DebugRenderAddGrid`, see notes.
    - [ ] Grid is clipped to area the camera can potentially see (allowing for an infinitly large grid)
- [ ] MeshUtils: `AddCylindarToIndexedVertexArray`
- [ ] MeshUtils: `AddConeToIndexedVertexArray`
- [ ] `RGBA LerpAsHSL( RGBA const &a, RGBA const &b, float t );` 


------
Controls

Tab - Autocomplete ( finish command or scroll through available commands when line is blank or command is present )
Shift Tab - Move backwards through Autocomplete suggestions
F2 - Cycle Sampler (between point and bilinear)
F3 - Cycle Blend Mode (Affects the debug image in the top middle)
WASD ( C and Spacebar for up down ) - Move camera
Shift - Move faster 

------
Dev Console Commands
set_mouse_sensitivity multiplier=NUMBER - change multiplier for mouse sensitivity
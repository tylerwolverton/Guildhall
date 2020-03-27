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

- [x] World Rendering
    - [x] Points
    - [x] Lines
    - [x] Arrows
        - Lines and arrows can have their ends colored differently and then a tint applied to the whole object instead of different start and end colors for each end of the line/arrow
    - [x] Basis
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
- [x] *X05.00: 03%*: MeshUtils: `AddCylinderToIndexedVertexArray`
- [x] *X05.00: 03%*: MeshUtils: `AddConeToIndexedVertexArray`

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

Debug Commands
Q - Draw world point at camera location XRAY
O - Draw world line from camera to box DEPTH
E - Draw Arrow from camera to box DEPTH
R - Draw world bounds DEPTH
T - Draw 3 world text objects at camera, pivoted to one, zero, and middle (.5, .5) respectively ALWAYS
Y - Draw wire sphere ALWAYS
B - Draw world billboard text and billboardf text in front of camera XRAY
U - Draw world quad at fixed location DEPTH
I - Draw basis at camera DEPTH
1 - Draw screen point in center of screen
2 - Draw screen line rom bottom left corner to middle of screen
3 - Draw screen arrow to the right and down
4 - Draw a screen quad in the bottom left corner roughly
5 - Draw a textured quad in the bottom left corner
6 - Draw screen textf in the middle top of the screen

------
Dev Console Commands
All the ones listed above in the assignment as well as
set_mouse_sensitivity multiplier=NUMBER - change multiplier for mouse sensitivity

Note: my dev console can't handle spaces in strings or parentheses around vec values for now
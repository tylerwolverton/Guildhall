Project: FPS

------

### Goal [100/100]
- [ ] Have a Quad, Sphere, and Cube rendering in world with normals and tangents;
    - [x] Make a new `Vertex_PCUTBN` or `VertexLit`
    - [x] Update or create new methods for generating these shapes, with normals/tangents/bitangents computed.
    - [x] Create meshes using this new vertex format.
    - [x] Be sure to update places where D3D11 needs to know format...
        - Setting vertex buffer needs to know correct stride
        - Creating an input layout needs the correct layout to tie it to the shader
    - [x] Be sure your vertex buffer stores the correct format for the vertices stored in it
- [ ] Be able to switch to cycle active shader to show the following...  Use keys `<` and `>` for this
    - [ ] Current shader and hotkeys are shown on screen using debug screen text.
    - [x] Normal lighting shader - `lit.hlsl`
    - [x] Diffuse color only (no lighting) - this is your `default` shader
    - [ ] Vertex Normals (transformed by model) - `normals.hlsl`
    - [ ] Vertex Tangents (transformed by model) - `tangents.hlsl`
    - [ ] Vertex Bitangents (transformed by model) - `bitangents.hlsl`
    - [ ] Surface Normals (uses TBN) - `surface_normals.hlsl`
- [-] Each object should be rotating around the `Y` and `X` axis at different speeds.  This is to correct a correct application to model matrices to TBN space.
- [-] `9,0` - Be able to adjust global ambient light
    - [ ] Console command `light_set_ambient_color color=rgb` to set ambient color to whatever you want
- [-] There should be one point light in the scene that you can adjust.
    - [ ] Be able to toggle attentuation using `T`
        - [ ] Default to `linear attenuation`, or (0, 1, 0)
        - [ ] Cycle from linear -> quadratic -> constant, ie (0,1,0) -> (0,0,1) -> (1,0,0)
        - *Note: Suggest adding a `light_set_attenuation` command that allows you to set it arbitrarily so you can see how it affects the light behaviour.*
    - [ ] Point light color and position is represented in world using a `DebugDrawWorld...` call.  Point or Sphere work well. 
        - [ ] Don't do this if the light is following the camera it is will just get in the way. 
    - [ ] Keyboard `-` and `+` should change its intensity
    - [ ] A console command `light_set_color color=rgb` to set the light color.  
    - [ ] Be able to switch the light's positioning mode... Keys are just suggestions, feel free to use whatever as long as it is in your readme. 
        - [ ] `F5` - Origin (light is positioned at (0,0,0))
        - [ ] `F6` - Move to Camera (light is moved to where the camera currently is, and stays there)
        - [ ] `F7` - Follow Camera (light is moved to camera each frame while in this mode)
        - [ ] `F8` - Animated.  Light follows a fixed path through the enfironment (cirlce your objects, or figure 8 through the objects)
- [ ] Be able to adjust object material properties...
    - [ ] `[,]` keys should adjust specular factor between 0 and 1 (how much does specular light show up
    - [ ] `',"` keys should adjust specular power (min of 1, but allow it to go as high as you want.

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

------
Dev Console Commands
All the ones listed above in the assignment as well as
set_mouse_sensitivity multiplier=NUMBER - change multiplier for mouse sensitivity

Note: my dev console can't handle spaces in strings or parentheses around vec values for now
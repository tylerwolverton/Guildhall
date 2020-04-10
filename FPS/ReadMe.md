Project: FPS

------

### Goal [80/80]
- [ ] Be able to support up to at least 8 concurrent lights
- [ ] Be able to support different light types...
    - [ ] Point Light...
        - [ ] Local 
        - [ ] Infinite
    - [ ] Infinite Directional Light...
    - [ ] Spot Light
       - [ ] Local
       - [ ] Infinite
    - *Have all these visible in the scene concurrently*
    - *Note: Local and Infinite are just attenuation values...*
- [ ] Shader should be branchless using a unified lighting structure 
- [ ] Support linear fog...
      - [ ] `RenderContext::EnableFog( float nearFog, float farFog, rgba nearFogColor, rgba farFogColor );` 
      - [ ] `RenderContext::DisableFog()` (shader is just 'always' going to do fog, so what do you set this to to make it not have an effect?)
- [ ] Dissolve Shader Effect
    - [ ] Support a `Material Block` uniform buffer with your `RenderContext`
        - Suggest a `RenderContext::SetMaterialBuffer( RenderBuffer* buf )`  
    - [ ] When binding a dissolve pattern to use, be sure to use a free slot.   
        - Suggest having a slot titled `USER_SLOT_START`, defined to likely 8.  That way when binding a texture 
          a user can just say `USER_SLOT_START + idx` for the slot they want.  
          You can also move your common textures (diffuse, normal, et.al.) to a later slot if you like users starting at 0. 
    - [ ] Have your dissolve shader expose the following controls...
        - [ ] A dissolve "depth" or value set to 0 to 1 to control how dissolved something is.
        - [ ] A dissolve "range" to give a *burned edge* to the dissolve effect.  This is the range near the edge at which is interpolates between burn_start_color and burn_end_color.  
        - [ ] A burn start color.
        - [ ] A burn end color
        - *Note, the full range you will be moving through with this effect is not 0 to 1, but `(1 + 2 * dissolve_range)` (why?).  You can think of it kinda like a scroll bar.  Meaning - how does the `dissolve_value` or depth that is 0 to 1 actually affect our dissolve range we're using in the shader?*

### Extras
- [ ] *X07.10: 05%*: Triplanar Shader
- [ ] *X07.11: 05%*: Projected Texture Effect
- [ ] *X07.12: 05%*: Interior Mapping Shader
- [ ] *X07.20: 05%*: Parallax Mapping Shader
    - [ ] *X07.21 05%*: Self Occluding Parallax Shader, aka Deep/Steep Parallax Shader
- [ ] *X07.30: 05%*: Support a Cube Map
    - [ ] *X07.31: 05%*: Support skybox clearing mode for a camera
    - [ ] *X07.32: 03%*: Support reflections into a skybox (can use specular factor to determine how shiny something is)
 - [x] X02.10 : 02pts: Shader Reloading. On key press (F4), reload all shaders in your shader database

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
light_set_ambient_color color=rgb - passed in as 0-1,0-1,0-1
light_set_color color=rgb - passed in as 0-1,0-1,0-1

Note: my dev console can't handle spaces in strings or parentheses around vec values for now
Project: FPS

------

### Goal [80/80]
- [x] Be able to support up to at least 8 concurrent lights
- [x] Be able to support different light types...
    - [x] Point Light...
        - [x] Local 
        - [x] Infinite
    - [x] Infinite Directional Light...
    - [x] Spot Light
       - [x] Local
       - [x] Infinite
    - *Have all these visible in the scene concurrently*
    - *Note: Local and Infinite are just attenuation values...*
- [x] Shader should be branchless using a unified lighting structure 
- [x] Support linear fog...
      - [x] `RenderContext::EnableFog( float nearFog, float farFog, rgba nearFogColor, rgba farFogColor );`
        - Note: I only use 1 color for fog 
      - [x] `RenderContext::DisableFog()` (shader is just 'always' going to do fog, so what do you set this to to make it not have an effect?)
- [x] Dissolve Shader Effect
    - [x] Support a `Material Block` uniform buffer with your `RenderContext`
        - Suggest a `RenderContext::SetMaterialBuffer( RenderBuffer* buf )`  
    - [x] When binding a dissolve pattern to use, be sure to use a free slot.   
        - Suggest having a slot titled `USER_SLOT_START`, defined to likely 8.  That way when binding a texture 
          a user can just say `USER_SLOT_START + idx` for the slot they want.  
          You can also move your common textures (diffuse, normal, et.al.) to a later slot if you like users starting at 0. 
    - [x] Have your dissolve shader expose the following controls...
        - [x] A dissolve "depth" or value set to 0 to 1 to control how dissolved something is.
        - [x] A dissolve "range" to give a *burned edge* to the dissolve effect.  This is the range near the edge at which is interpolates between burn_start_color and burn_end_color.  
        - [x] A burn start color.
        - [x] A burn end color
        - *Note, the full range you will be moving through with this effect is not 0 to 1, but `(1 + 2 * dissolve_range)` (why?).  You can think of it kinda like a scroll bar.  Meaning - how does the `dissolve_value` or depth that is 0 to 1 actually affect our dissolve range we're using in the shader?*

### Extras
- [x] *X07.10: 05%*: Triplanar Shader
- [x] *X07.11: 05%*: Projected Texture Effect
    - Note: Bound to light 0 position, can be placed with F6 or F7 like other lights
- [x] *X07.13: 03%*: Fresnel or Highlight Shader
- [x] X02.10 : 02pts: Shader Reloading. On key press (F4), reload all shaders in your shader database

------
I set up 8 lights in my scene that are on by default, cycle with arrow keys and hit enter to turn off
Default Lights
0 - Finite Spot light following camera
1 - Finite point (red)
2 - Infinite point (blue)
3 - Infinite Directional (green arrow) (white light)
4 - Infinite spot (red arrow) (purple light)
5 - Finite point (green)
6 - Finite point (yellow)
7 - Looping finite point (orange)

Also, I have the projector for projection shader bound to light 0, so it can be positioned with F6, F7 just like the other lights

------

Controls

Game
F2 - Cycle Sampler (between point and bilinear)
F3 - Cycle Blend Mode (Affects the debug image in the top middle)
F4 - Reload all shaders
WASD ( C and Spacebar for up down ) - Move camera
Shift - Move faster 

Debug Commands

------
Dev Console Commands
set_mouse_sensitivity multiplier=NUMBER - change multiplier for mouse sensitivity
light_set_ambient_color color=rgb - passed in as 0-1,0-1,0-1
light_set_color color=rgb - passed in as 0-1,0-1,0-1

Note: my dev console can't handle spaces in strings or parentheses around vec values for now
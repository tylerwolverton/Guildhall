Project: FPS

------

## Tasks
- [x] `NamedProperties`
    - [x] Switch named properties over to use `TypedProperty` instead of just strings
    - [x] Add ability to subscribe methods to your event system. 
    - [x] Add ability to unsubscribe an object from the event system (unsubscribes all methods on that object)

- [x] Color Transform (ex: Grayscale) Effect
    - [x] Create/Recycle a color target matching your swapchain's output.
    - [x] Render game as normal
    - [x] Create/Recycle another match target
    - [x] Apply an effect on the original image, outputting to this new texture
        - [x] Effect applies a per-pixel color transform using a matrix. 
        - [x] Make it so you can specify a "strength" for this transform, where 0 
              has no effect, and 1 is the full effect so you can blend between them.
        - [x] Be able to specify a tint and tint power that pixels trend toward (useful for fades)
              - `0` would have no effect
              - `1` the resultant color would be the tint 
        - [ ] **Optional Challenge: Have all the above be done with a single mat44.**
    - [x] Copy this final image to the swapchain before present
   
- [-] Bloom Effect
    - [x] Set your normal color target, and a secondary "bloom" target
        - [x] Camera can set set tertiary render targets
        - [x] Shader has a secondary output specified
    - [-] When done, be able to blur the bloom target
        - [x] Create/Recycle a matching color/render target
        - [ ] Run a guassian blur pass N times, each pass consisting of one horizontal and one vertical pass
            Note: Currently using a box blur instead
            - [ ] Each step in a pass will swap out the src/dst target and render a full screen blur shader
                - [ ] Run once horizontally
                - [ ] Run once vertically
    - [x] Take the result of the blur, and the normal color output, and combine them
          into the final image.
    - [x] Be able to toggle blur on-and-off to see it working or not
        - [x] Disabling the blur just doesn't run the blur and composite steps;

- [x] Texture Pool
    - [x] Be able to ask your `RenderContext` for a temporary render target of a given resolution and size.
        - [x] Search for an existing free texture first, and return it if you find one.
        - [x] If there are none available, create and return a new one.
    - [x] Be able to relinquish back these temporary textures back to the `RenderContext` when you're done with them.
        - This will allow them to be reused.
    - [x] Add a `RenderContext::GetTotalTexturePoolCount()` to tell you how many textures have been created this way.
    - [x] Add a `RenderContext::GetTexturePoolFreeCount()` to tell you how many are currently in the pool to be recycled
    - [x] Debug render these counts to the screen to you can make sure you're properly recycling during this assignment.
        - At any given time you likely will not have more than 3 textures in use at once, meaning your pool should never exceed that count.  This can really depend on your scene though.  For eaxmple, in this assignment for bloom... 
          1. Camera color output
          2. Camera bloom target
          3. Temporaries
             - Blurring secondary output
             - Composite output 

-------
Notes

-I print out the result of cleaning the scifi_fighter to the dev console on startup
-Running in debug takes a really long time to load since I added in a second obj for the demo. I'm guessing my obj loading needs to be optimized, but release should work fine.

-------
Controls

-------
Game
F4 - Reload all shaders
WASD ( Spacebar and C for up down ) - Move camera
Shift - Move faster 

------
Dev Console Commands
set_mouse_sensitivity multiplier=NUMBER - change multiplier for mouse sensitivity
light_set_ambient_color color=rgb - passed in as 0-1,0-1,0-1
light_set_color color=rgb - passed in as 0-1,0-1,0-1
unsubscribe_game - unsubscribe the light_set effects (can't be re-enabled)

Note: my dev console can't handle spaces in strings or parentheses around vec values for now
Project: FPS

## Core Task List

- [x] *15pts*: Get a Working Orthographic Camera
    - [x] *05pts*: Implement `MakeOrthographicProjectionMatrixD3D` in `Engine/Math/MatrixUtils.hpp`, `.cpp`.  
    - [x] *05pts*: Camera now owns a `UniformBuffer` for their matrices
    - [x] *05pts*: Camera updates and binds their uniform buffer in `BeginCamera`
- [x] *18pts*: Get Working Textures
    - [x] *04pts*: Be able to create a `Texture` from an Image/File
    - [x] *04pts*: `TextureView` supports shader resource views (srv)
    - [x] *04pts*: Make a `Sampler` class
    - [x] *03pts*: Default Samplers (Linear+Wrap, Point+Wrap)
    - [x] *03pts*: `RenderContext::GetOrCreateTexture` works again.
- [x] *08pts*: Blend States supported by shader.
    - [x] *04pts*: Blend states creating when renderer starts up.
    - [x] *04pts*: ADDITIVE, OPAQUE, and ALPHA blend modes supported
- [x] *05pts*: Default Built-In Shader to use when `BindShader(nullptr)` is used
    - [x] `Renderer::SetBlendState` should affect this shader; 
- [x] *05pts*: Render two textured quads on screen
    - [x] *04pts*: One using an invert color shader
    - [x] *01pts*: One default
- [x] *34pts*: Dev Console
    - [x] *02pts*: User can open the dev console using the **tilde** key '\~'.  
                   You may use a custom key if you make a note in your assignment readme.
        - [x] Upon opening console, be sure the input is cleared.
    - [x] *02pts*: Pressing tilde again closes the console.
    - [x] *09pts* If console is open, Escape should...
        - [x] *03pts*: Clear input if there is any text currently there
        - [x] *03pts*: Close the console if console is open.
        - [x] *03pts*: Normal game behaviour otherwise (for example, for now it will quit your application)
    - [x] *14pts*: User can type commands **only** while console is open.
        - [x] *02pts*: Typing should insert characters at the carot
        - [x] *02pts*: The carot blinks (every quarter to half second usually looks okay)
        - [x] *02pts*; Left & Right arrow keys should move carot forward and back along string 
        - [x] *02pts*: Delete should delete the character after the carot
        - [x] *02pts*: Backspace should delete the character before the carot
        - [x] *02pts*: Pressing `Enter` submits the command
        - [x] *02pts* If it is an invalid command (it has not handler), print an error message
    - [x] *02pts* Game input is ignored while dev console is open.
    - [x] *03pts*: Support a `quit` command that quits the game.
    - [x] *03pts*: Support a `help` command that lists all exposed commands
   
------

## Extras

- [x] *X03.01 : 03pts*: Built-in error shader to use when a shader fails to compile.   
- [x] *X03.02 : 04pts*: AutoComplete
        - Used windows cmd behavior as a model, implemented both Tab and Tab+Shift to move backwards
        - Added HelpTester and HelpTester2 commands to prove autocomplete works, I'll take them out after this assignment
- [x] *X03.04 : 04pts*: Command History
- [x] *X03.05 : 03pts*: Persistant Command History (requires X03.02)
- [-] *X03.10 : 04pts*: Cut, Copy & Paste from Window's Clipboard (Requires X03.04)
        - Paste only for now since I don't have text selection
- [-] *X03.24 : 02pts*: Navigation Hotkeys, Home, End, Ctrl + Arrows 
        - Only Home and End, no Ctrl+Arrows


Controls

Tab - Autocomplete ( finish command or scroll through available commands when line is blank or command is present )
Shift Tab - Movebackwards through Autocomplete suggestions
F2 - Cycle Sampler (between point and bilinear)
F3 - Cycle Blend Mode (Affects the debug image in the top middle)
Arrows - Move camera during game only
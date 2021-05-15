Project: ModelViewer

-------
Controls

-------
Game
F4 - Reload all shaders
WASD ( E and Q for up down ) - Move camera
Shift - Move faster 

( All others on screen )

------
Dev Console Commands
set_mouse_sensitivity multiplier=NUMBER - change multiplier for mouse sensitivity
light_set_ambient_color color=rgb - passed in as 0-1,0-1,0-1
light_set_color color=rgb - passed in as 0-1,0-1,0-1
unsubscribe_game - unsubscribe the light_set effects (can't be re-enabled)

Note: my dev console can't handle parentheses around vec values
-------

Commandlets
-------
run from cmd line:
ModelViewer_x64.exe cookObj Models/Bunny

cooks obj file with given path to .twsm in same directory
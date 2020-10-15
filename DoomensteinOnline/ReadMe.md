Project: Doomenstein Online
A 3D first person shooter in the style of Wolfenstein 3D and Doom

Controls

Game
Esc - Quit
~ - Open dev console
F1 - Toggle debug drawing
F3 - Possess/Unpossess entity
F4 - Reload all shaders
WASD - Move camera forward/back/sideways
QE - Move camera Up/Down
Shift - Move faster 

Debug Commands

------
Dev Console Commands
start_multiplayer_server port=<port number>
connect_to_multiplayer_server ip=<"ip address"> port=<port number> -- NOTE: Since I'm not updating RemoteServer from Authoritative yet, connecting to multiplayer server will be missing tiles and animations since the update has not been implemented

set_mouse_sensitivity multiplier=NUMBER - change multiplier for mouse sensitivity
warp map=desinationMapName pos=float,float yaw=float
How to Use
- Run Zephyr_x64 to start the game. 

Keyboard
- WASD to move the player
- Arrow keys to fire projectile in corresponding direction
- Esc -> Pause, press again to Exit game
- ~ -> Open dev console

Debug Commands
- F2 -> Fire debug event ( change blob text )
- F5 -> Reload all data files and restart game
- F6 -> Reload all script files only (warning, doing this can break certain script interactions like the door in level 1. The main use of this might be to debug some AI in a level, not to test larger game management functionality)

Scripts
Data\Scripts -> Location for all .zephyr files

Any changes made to the scripts will be reflected when returning to the main menu.
Supported features
- StateMachine
- State
- Number variables
- String variables
- Vec2 variables
- Arithmetic operators
- Logical operators
- ChangeState(<targetState>) function
- if statements
- FireEvent(<parameters>)
- OnEvent(<event name>)
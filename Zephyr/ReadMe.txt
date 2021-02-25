How to Use
- Run Zephyr_x64 to start the game. 

Keyboard
- WASD to move the player
- Arrow keys to fire projectile in corresponding direction
- Esc -> Pause, press again to Exit game
- ~ -> Open dev console

Debug Commands
- F5 -> Reload all data files and restart game
- F6 -> Reload all script files only (warning, doing this can break certain script interactions like the door in level 1. The main use of this might be to debug some AI in a level, not to test larger game management functionality)

Scripts
Data\Scripts -> Location for all .zephyr files

VS Code Syntax Highlighting
- Copy the Zephyr/zephyr-lang directory into <user directory>\.vscode\extensions

Doxygen
- The Zephyr/Documentation folder has doxygen generated html docs for ( about half, more coming ) GameAPI methods

Any changes made to the scripts will be reflected when returning to the main menu.
Supported features
- State
- Number variables
- String variables
- Vec2 variables
- Arithmetic operators
- Logical operators
- ChangeState(<targetState>) function
- if statements
- Function(<parameters>)
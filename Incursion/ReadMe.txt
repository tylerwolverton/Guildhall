Known Issues
- None

How to Use
- Run Incursion_x64 to start the game. 

Controller
- Left stick moves the tank
- Right stick rotates tank gun
- 'A' button fires a bullet

Keyboard
- WASD or Arrow keys move the tank
- Space fires a bullet

- Esc -> Exit game
- P -> Pause game
- T -> Slow game down to 1/10th speed
- Y -> Increase game speed x4
- F1 -> Toggle degub rendering for entities
- F3 -> Toggle NoClip
- F4 -> Toggle debug camera
- F5 -> Teleport to next map
- F8 -> Restarts the game

Bonus Features
- Health bars
- Screen shake on entity death
- Water tiles that block movement but not vision or bullets
- A death counter to keep track of deaths per each run

Deep Learning
- Working on Incursion Alpha taught me the benefit of building "dev scaffolding" into a project to allow for faster testing and iteration. In the past everything I've written for a project has been related to the features themselves or long term testing (like debug rendering). Incursion was getting large and long enough that driving to the end of level 1 so I could test level 2's worm generation was a pain, so I added my own debug command to jump maps. I also used some debug tile drawing to tint tiles during my flood fill testing that I hacked in and then removed once it was stable. This approach supports one of my earlier deep learnings, "code is cheap" in that I need to be more willing to experiment and throw out code, rather than only writing my final game code. I plan to build my testing code sooner in future projects and be more willing to spend a little time writing a tool that will make iteration and testing faster throughout the project.
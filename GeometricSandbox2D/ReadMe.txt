Known Issues
- If a file contains a different scene size than my 16x9 default, my quad tree will not cover the new area and checks with a quad tree will fail. If the new area is larger then any objects in the area outside the initial quad tree will never return collisions while quad tree checking is turned on.

How to Use
- Run GeometricSandbox2D_x64 to start the game. 

Console Commands
- save_ghcs_file name=<name of file relative to Run/Data> Save the current convex scene as a .ghcs file.
- load_ghcs_file name=<name of file relative to Run/Data> Load a convex scene from a .ghcs file

Keyboard (On screen)
- Esc -> Exit game
- ~ -> Open dev console
- [, .] halve/double object count 
- [N M] halve/double Raycast Count
- [S E] + [Mouse Drag] Move start and end points of visible raycast
- [W R] Rotate object under mouse
- [K L] Scale object under mouse 
- [F8] Reload Game
- [F1] Enable debug drawing
- [F2] Change Broadphase check
- [F3] Change Narrowphase check

From Assignment 1's performance analysis

Summary of Findings
1. How many raycasts/ms can you do vs. 100 objects?  1000?  10000?
	Release mode, Quad Tree depth 3 
	time given to complete raycast operations each frame in ms and used to calculate rays per ms in parentheses

	128 objects 
	1024 Raycasts
	- No optimization: 5 ms (204/ms)
	- Bounding Disc: 2.8 ms (366/ms)
	- Quad Tree:       3 ms (341/ms)
        - Both:          2.8 ms (366/ms)

	1024 objects
	1024 Raycasts
	- No optimization: 18 ms ( 57/ms)
	- Bounding Disc:   10 ms (102/ms)
	- Quad Tree:        6 ms (171/ms)
        - Both:           5.8 ms (177/ms)

	16384 objects
	1024 Raycasts
	- No optimization: 120 ms ( 8.5/ms)
	- Bounding Disc:    55 ms (18.6/ms)
	- Quad Tree:        50 ms (20.5/ms)
        - Both:             45 ms (22.8/ms)

	1024 objects
	16384 Raycasts
	- No optimization: 280 ms ( 3.7/ms)
	- Bounding Disc:   146 ms ( 7  /ms)
	- Quad Tree:        90 ms (11.4/ms)
        - Both:             82 ms (12.5/ms)

2. How does this improve (or worsen!) when you enable your hashing/partitioning scheme?
	Bounding discs always provide about a 2x speed increase, while the Quad Tree has small increases with small object counts, but becomes more effective as the object count grows. Combining the two is only a slight advantage compared to whichever is the most effective strategy for a given object size.

3. How do these speeds compare in each build configuration (Debug, DebugInline, FastBreak, Release)?
	Debug
	1024 objects
	1024 Raycasts
	- No optimization: 650 ms
	- Bounding Disc:   340 ms
	- Quad Tree:       250 ms
        - Both:            250 ms

	Release
	1024 objects
	1024 Raycasts
	- No optimization: 18 ms
	- Bounding Disc:   10 ms
	- Quad Tree:        6 ms
        - Both:           5.8 ms

	I found Debug mode to be very much slower than release, as shown in this side by side comparison. In Debug, getting up to 64 objects makes the app almost unusable, since the fps is so low that hotkeys can be missed

4. Any general trends you can observe, i.e. the speed seems to be O(N) or O(N2) with #objects, #rays, etc.
	Raycasts seem to linearly decrease the speed, doubling the number of raycasts doubles the raycast time. Object count is a little different. At smaller object counts (< 1024) it seems linear as the raycast time doubles as the objects double, but then as the object count grows higher the raycast time increased but by less than half, similr to a logarithmic pattern.

5. Any data specific to your hashing/partitioning scheme you can observe (e.g. AABB Tree depth)
	After comparing some different quad tree depths, I saw that depth 3 was the most efficient (with 1024 objects and large amounts of raycasts). Depth 4 was actually the same when using only 1024 raycasts, but as the raycast count increased 4 became slower than 3. Higher tree depths actually dropped the speed, with a dpeth of 7 seeming to do nothing to speed up the raycasts. I suspect this is because my implementation allows objects to be in multiple leaf nodes, and then it only processes nodes once when the list to process is returned. Higher and lower depths likely result in a lot more potential matches for the raycast which doesn't prune the results as much.

6. Anything else interesting you observe about your results
	Not really about raycasts, but I switched my rendering methods to append verts and do 1 draw call instead of individual draw calls for everything and it massively increased by performance, especially with debug rendering on. There's no way I would've been able to even run the game with debug on at 1024 objects and 1024 raycasts.

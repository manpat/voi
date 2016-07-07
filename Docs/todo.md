Shit To Do
==========

- Rendering
	- Mirrors
- Physics
	- Colliders
	- Collider info
		- Kinematic
		- Configurable sizes in blender
- Lua
	- Enumerate entities
	- Control entities
		- Movement
	- Trigger audio events

	- Event scheduling?

- Config
	- Render stuff
		- UI size? á la minecraft
		- AA
	- Audio stuff
		- Volume
		- Quality?
			- This can control sample rate and wave table granularity
	- Save config after modification in game
- Sound
	- Lua synth def
	- Sound through portals
- UI
	- Menu system
		- Main menu
		- Settings
		- Pause menu

		- Needs clicky buttons
		- Needs slidy sliders
		- Checkboxes
		- Dropboxes?
		- Animations?
		- Text rendering!
- Save/Load

- Optimise
	- The glDrawElements in RenderMesh takes up most of frame time
		- The way submeshes are handled should probably change
	- Single vertex buffer
	- Bake static geometry into single Mesh/Entity
	- Vert sorting algorithm seems kinda slow
		- Simple solution: Move it into voi_scene_exporter.py

	- Optional flag for portals that bails from recursion
	- Improve portal visibility algorithm 
Shit To Do
==========

- Rendering
	- Mirrors
	- Portal clipping
- Physics
	- Colliders
	- Collider info
		- Kinematic
		- Configurable sizes in blender
	- Portal interaction
- Lua
	- Enumerate entities
	- Control entities
		- Movement
		- Recieve frob events
	- Trigger audio events
	- Change fog settings, etc.

- Config
	- Window stuff
		- Size, fullscreen
	- Render stuff
		- UI size? á la minecraft
		- AA
	- Audio stuff
		- Volume
		- Quality?
			- This can control sample rate and wave table granularity
	- Save/load config
		- Would be easy to do manually and would make catching errors easier
		- Using lua means less code potentially but is overkill
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
- EntityManager

- Optimise
	- Single vertex buffer
	- Bake static geometry into single Mesh/Entity
	- Vert sorting algorithm seems kinda slow
		- Simple solution: Move it into voi_scene_exporter.py
Scripting
=========

Entities marked as 'interactive' can be assigned a callback in blender to be run when frobbed.
Callbacks must be formatted like so, `filename.lua:callback_name`. 
Global state in a script can be shared between callbacks in the same file, for entities in the same scene.

Scripts are searched for in the 'scripts' directory.

API Reference
-------------
- Vector
	- Arithmetic `+, -, *, /` all perform component-wise operations
	- Unary `-` works as expected
	- `vec(x,y,z)` - Construct a 3 component vector containing from (x,y,z)
	- `vec(v)` - Equivalent to `vec(v,v,v)`
	- `vec()` - Equivalent to `vec(0)`

	- `vec.x(v), vec.y(v), vec.z(v)` - Returns components of `v`
	- `vec.x(v,a), vec.y(v,a), vec.z(v,a)` - Sets components of `v` to `a`, and returns `v`
	- `vec.normalize(v)` - Returns a normalized copy of `v`
	- `vec.length(v)` - Returns the magnitude of `v`
	- `vec.dot(a,b)` - Returns the dot product of `a` and `b`
	- `vec.cross(a,b)` - Returns the cross product of `a` and `b`
	- Note: Above functions can be rewriten as methods
		- `vec.function(v, a) -> v:function(a)`
		- This is particularly useful for the component accessors `vec:x(), ...`

- Random
	- `rand.ball(r)` - Generates a vec representing a point in a sphere of radius `r`
	- `rand.spherical(r)` - Generates a vec representing a point on a sphere of radius `r`
	- `rand.linear(min,max)` - Generates a value between `min` and `max`
		- `min` and `max` can be either scalar or vector
	- `rand.gauss(mean,deviation)` - Generates a gaussian distribution from `mean` and `deviation`
		- `mean` and `deviation` can be either scalar or vector

- Effects
	- `effects.fog(color, distance, density[, duration])` - Begins an interpolation toward specified fog settings over `duration` seconds
	- `effects.fog_color(color[, duration])` - Begins interpolating fog color toward `color` over `duration` seconds
	- `effects.fog_density(density[, duration])` - Begins interpolating fog density toward `density` over `duration` seconds
	- `effects.fog_distance(distance[, duration])` - Begins interpolating fog distance toward `distance` over `duration` seconds
	- `effects.vignette(intensity[, duration])` - Begins interpolating vignette intensity toward `intensity` over `duration` seconds

- Debug
	- `debug.point(pos[, color])` - Attempts drawing a debug point at `pos` using `color` if provided
	- `debug.line(p0, p1[, color])` - Attempts drawing a debug line from `p0` to `p1` using `color` if provided
	- `debug.line(p0, p1, c0, c1)` - Attempts drawing a debug line from `p0` to `p1` using the gradient `c0` -> `c1`

- Entity
	- `entity.lookup(id)` - Tries to find an entity with ID `id`
	- `entity.lookup(name)` - Tries to find an entity with name `name`
	- `entity.name(ent)` - Returns the name of entity `ent`
	- `entity.id(ent)` - Returns the id of entity `ent`
	- `entity.type(ent)` - Returns the entity type of entity `ent` in integer form
	- `entity.type_name(ent)` - Returns the entity type of entity `ent` in string form
	- `entity.pos(ent)` - Returns the position of entity `ent`
	- `entity.size(ent)` - Returns the unrotated bounding box size of entity `ent`
	- `entity.hidden(ent)` - Returns whether or not entity `ent` is hidden
	- `entity.layers(ent)` - Returns the layers that entity `ent` occupies
		- This can return multiple values
		
	- `entity.set_hidden(ent, h)` - Sets whether entity `ent` is hidden
	- `entity.set_layers(ent, ...)` - Sets the layers that entity `ent` occupies
		- Example invocation: `ent:set_layers(0, 2)` - causes ent to occupy layers 0 and 2

	- `entity.move_to(ent, target[, duration])` - Queues entity `ent` for an animation of movement towards vec `target` optionally over `duration` seconds (defaults to 1 second)

	- Note: Above functions can be rewriten as methods
		- `entity.function(e, a) -> e:function(a)`
#!BPY

import bpy
import sys
import math
import mathutils
from bpy.props import *

class ObjectPanel(bpy.types.Panel):
	bl_label = "Anomalia Properties"
	bl_idname = "OBJECT_PT_anomalia"

	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "object"

	def draw(self, context):
		layout = self.layout

		row = layout.row()
		row.prop(context.active_object, "anom_objecttype")

		row = layout.row()
		row.prop(context.active_object, "anom_invisible")

		row = layout.row()
		row.prop(context.active_object, "anom_layer")

		otyp = context.active_object.get("anom_objecttype", 0)

		if(otyp in [0,3,4,9,10]):
			row.prop(context.active_object, "anom_hidden")

		# Portal
		if(otyp == 1):
			row.prop(context.active_object, "anom_portaldst")

		# Interactible / Generic Trigger
		elif(otyp == 3 or otyp == 5):
			row = layout.row()
			row.prop(context.active_object, "anom_targetentity")
			row.prop(context.active_object, "anom_interactaction")

		# Bell
		elif(otyp == 10):
			layout.row().prop(context.active_object, "anom_bellnumber")
			layout.row().prop(context.active_object, "anom_targetentity")

		# Door
		elif(otyp == 4):
			layout.row().prop(context.active_object, "anom_doorcount")
			layout.row().prop(context.active_object, "anom_doorordered")
			layout.row().prop(context.active_object, "anom_doormovetime")

		# Halflife Point
		elif(otyp == 6):
			layout.row().prop(context.active_object, "anom_newarea")

		if(len(context.selected_objects) > 1):
			layout.row().operator("anomalia.consistentiser")

class SpeakerPanel(bpy.types.Panel):
	bl_label = "Anomalia Speaker Properties"
	bl_idname = "SPEAKER_PT_anomalia"

	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "object"

	@classmethod
	def poll(cls, context):
		return context.object is not None

	def draw(self, context):
		layout = self.layout
		ob = context.active_object

		layout.row().prop(ob, "anom_soundtype")

		styp = ob.get("anom_soundtype", 0)

		if(styp == 1):
			layout.row().prop(ob, "anom_soundpath")
		elif(styp == 2):
			layout.row().prop(ob, "anom_soundsynth")
			layout.row().prop(ob, "anom_soundreverb")
			layout.row().prop(ob, "anom_soundmix")

		layout.row().prop(ob, "anom_soundsize")

		if(len(context.selected_objects) > 1):
			layout.row().operator("anomalia.speaker_consistentiser")

class WorldPanel(bpy.types.Panel):
	bl_label = "Anomalia Scene Properties"
	bl_idname = "SCENE_PT_anomalia"

	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "world"

	def draw(self, context):
		world = context.world

		layout = self.layout
		layout.row().prop(world, "anom_skycolor")
		layout.row().prop(world, "anom_ambientcolor")
		layout.row()
		layout.row().prop(world, "anom_fogtype")
		layout.row().prop(world, "anom_fogcolor")
		
		fogtype = world.get("anom_fogtype", 1)

		if(fogtype == 1):
			row = layout.row()
			row.prop(world, "anom_foglinearstart")
			row.prop(world, "anom_foglinearend")

		elif(fogtype == 2 or fogtype == 3):
			layout.row().prop(world, "anom_fogdensity")

		if(bpy.data.objects.get("Player") is None):
			layout.row().operator("anomalia.spawner")

#
# 	  ,ad8888ba,
# 	 d8"'    `"8b                                                 ,d
# 	d8'        `8b                                                88
# 	88          88 8b,dPPYba,   ,adPPYba, 8b,dPPYba, ,adPPYYba, MM88MMM ,adPPYba,  8b,dPPYba, ,adPPYba,
# 	88          88 88P'    "8a a8P_____88 88P'   "Y8 ""     `Y8   88   a8"     "8a 88P'   "Y8 I8[    ""
# 	Y8,        ,8P 88       d8 8PP""""""" 88         ,adPPPPP88   88   8b       d8 88          `"Y8ba,
# 	 Y8a.    .a8P  88b,   ,a8" "8b,   ,aa 88         88,    ,88   88,  "8a,   ,a8" 88         aa    ]8I
# 	  `"Y8888Y"'   88`YbbdP"'   `"Ybbd8"' 88         `"8bbdP"Y8   "Y888 `"YbbdP"'  88         `"YbbdP"'
# 	               88
# 	               88

class WORLD_OT_spawner(bpy.types.Operator):
	bl_idname = "anomalia.spawner"
	bl_label = "Make Player Spawner"
	bl_description = "Creates a proxy object that specifies player spawn position and orientation"

	def execute(self, context):
		scene = bpy.context.scene
		obj = bpy.data.objects.new("Player", None)

		cam = bpy.data.cameras.new("PlayerCam")
		ocam = bpy.data.objects.new("PlayerCam", cam)

		ocam.location = (0, 0, 2.4)
		ocam.rotation_euler = (math.pi/2, 0, 0)
		ocam.parent = obj

		scene.objects.link(obj)
		scene.objects.link(ocam)

		return {'FINISHED'}

class OBJECT_OT_anomaliaconsistentiser(bpy.types.Operator):
	bl_idname = "anomalia.consistentiser"
	bl_label = "Make Consistent"
	bl_description = "Copies attributes from last selected object to all other selected objects"

	def execute(self, context):
		ao = context.active_object
		for ob in context.selected_objects:
			if(ob == ao or ob.type != 'MESH'):
				continue

			ob.anom_layer = ao.anom_layer
			ob.anom_hidden = ao.anom_hidden
			ob.anom_newarea = ao.anom_newarea
			ob.anom_invisible = ao.anom_invisible
			ob.anom_portaldst = ao.anom_portaldst
			ob.anom_objecttype = ao.anom_objecttype
			ob.anom_bellnumber = ao.anom_bellnumber
			ob.anom_targetentity = ao.anom_targetentity
			ob.anom_interactaction = ao.anom_interactaction
			ob.anom_doormovetime = ao.anom_doormovetime
			ob.anom_doorordered = ao.anom_doorordered
			ob.anom_doorcount = ao.anom_doorcount

		return {'FINISHED'}

class OBJECT_OT_speakerconsistentiser(bpy.types.Operator):
	bl_idname = "anomalia.speaker_consistentiser"
	bl_label = "Make Consistent"
	bl_description = "Copies speaker attributes from last selected speaker to all other selected speakers"

	def execute(self, context):
		ao = context.active_object
		for ob in context.selected_objects:
			if(ob == ao):
				continue

			ob.anom_soundmix = ao.anom_soundmix
			ob.anom_soundtype = ao.anom_soundtype
			ob.anom_soundpath = ao.anom_soundpath
			ob.anom_soundsize = ao.anom_soundsize
			ob.anom_soundsynth = ao.anom_soundsynth
			ob.anom_soundreverb = ao.anom_soundreverb

		return {'FINISHED'}

#
# 	88b           d88            88
# 	888b         d888            ""
# 	88`8b       d8'88
# 	88 `8b     d8' 88 ,adPPYYba, 88 8b,dPPYba,
# 	88  `8b   d8'  88 ""     `Y8 88 88P'   `"8a
# 	88   `8b d8'   88 ,adPPPPP88 88 88       88
# 	88    `888'    88 88,    ,88 88 88       88
# 	88     `8'     88 `"8bbdP"Y8 88 88       88
#
#

bl_info = {
	"name": "Anomalia Properties",
	"author": "Patrick Monaghan",
	"description": "Adds properties specific to the development of Anomalia",
	"category": "Object",
	"version": (0, 0, 1),
	"blender": (2, 6, 9),
}

# For synchronising with blenders layers
def layer_update(self, context):
	layer = self["anom_layer"]
	self.layers[layer] = True

	for l in range(20):
		self.layers[l] = (layer == l)

# For synchronising with blenders layers
def poll_object_layer(scene):
	obj = bpy.context.active_object
	if obj.type == 'MESH':
		for i, v in enumerate(obj.layers):
			if v and i != obj.anom_layer:
				obj.anom_layer = i
				break

def skycol_update(self, context):
	col = self["anom_skycolor"]
	context.world.horizon_color = col

def ambcol_update(self, context):
	col = self["anom_ambientcolor"]
	context.world.ambient_color = col

def register():
	obtypes = [
		('b', 'Bell', "A bell, yo'", '', 10),
		('v', 'Movable', 'An object that can be picked up', '', 9),
		('c', 'Checkpoint', 'A respawn point', '', 8),
		('e', 'Level Entry', 'Target area of a Halflife Point', '', 7),
		('l', 'Halflife Point', 'A trigger that loads a new area when entered', '', 6),
		('t', 'Trigger', 'A generic trigger', '', 5),
		('d', 'Door', 'An openable door', '', 4),
		('i', 'Interact', 'Is interactible. Triggers event(s) on interact', '', 3),
		('m', 'Mirror', 'Can reflect into another layer', '', 2),
		('p', 'Portal', "Act's as a portal between layers", '', 1),
		('_', 'World', '', '', 0),
	]

	obj = bpy.types.Object

	# General
	obj.anom_layer = IntProperty(name="Layer",
		default=0, min=0, max=10, subtype='UNSIGNED',
		update=layer_update)

	obj.anom_objecttype = EnumProperty(items=obtypes,
		name="Object Type", default='_')

	obj.anom_invisible = BoolProperty(name="Invisible", default=False)
	obj.anom_hidden = BoolProperty(name="Object Hidden", default=False)

	# Portal / Mirror
	obj.anom_portaldst = IntProperty(name="Destination Layer",
		min=0, max=10, default=1, subtype='UNSIGNED')

	# Interact / Trigger / Bell
	obj.anom_targetentity = StringProperty(name="Target Entity")
	obj.anom_interactaction = StringProperty(name="Action")

	# Bell
	obj.anom_bellnumber = IntProperty(name="Order in Sequence", min=0, max=20, default=0)

	# Area boundary
	obj.anom_newarea = StringProperty(name="Level Name")

	# Door
	obj.anom_doormovetime = FloatProperty(name="Move Time", min=0.5, max=10.0, default=2.0)
	obj.anom_doorordered = BoolProperty(name="Ordered Sequence")
	obj.anom_doorcount = IntProperty(name="Lock Count", min=1, max=30, default=1)

	sptypes = [
		('_', 'None', 'Not a sound', '', 0),
		('f', 'File', 'A sound loaded from a file', '', 1),
		('s', 'Synthesizer', 'A synthesized sound', '', 2),
	]

	obj.anom_soundtype = EnumProperty(items=sptypes, name="Speaker Type", default='_')
	obj.anom_soundpath = StringProperty(name="Sound Path")
	obj.anom_soundsynth = StringProperty(name="Synth Name")
	obj.anom_soundsize = FloatProperty(name="Sound Size", default=1.0, min=0.1, max=100.0)
	obj.anom_soundreverb = FloatProperty(name="Reverb Time", default=10000.0, min=10.0, max=20000.0)
	obj.anom_soundmix = FloatProperty(name="Reverb Mix", default=100.0, min=0.0, max=100.0)

	scn = bpy.types.World
	fogtypes = [
		('_', 'None', '', '', 0),
		('l', 'Linear', '', '', 1),
		('e', 'Exp', '', '', 2),
		('2', 'Exp2', '', '', 3),
	]

	scn.anom_skycolor = FloatVectorProperty(name="Sky Color", subtype='COLOR', 
		min=0.0, max=1.0, size=3, default=(0,0,0), update=skycol_update)

	scn.anom_ambientcolor = FloatVectorProperty(name="Ambient Color", subtype='COLOR', 
		min=0.0, max=1.0, size=3, default=(0,0,0), update=ambcol_update)

	scn.anom_fogtype = EnumProperty(items=fogtypes, name="Fog Type", default='l')
	scn.anom_fogcolor = FloatVectorProperty(name="Fog Color", subtype='COLOR', 
		min=0.0, max=1.0, size=3, default=(0,0,0))

	scn.anom_fogdensity = FloatProperty(name="Fog Density", min=0.0, max=1.0)
	scn.anom_foglinearend = FloatProperty(name="Fog Linear End", min=0.0)
	scn.anom_foglinearstart = FloatProperty(name="Fog Linear Start", min=0.0)

	bpy.app.handlers.scene_update_post.append(poll_object_layer)

	bpy.utils.register_module(__name__)

def unregister():
	scn = bpy.types.World
	obj = bpy.types.Object

	del scn.anom_skycolor
	del scn.anom_ambientcolor

	del scn.anom_fogtype
	del scn.anom_fogcolor
	del scn.anom_fogdensity
	del scn.anom_foglinearend
	del scn.anom_foglinearstart

	del obj.anom_layer
	del obj.anom_hidden
	del obj.anom_newarea
	del obj.anom_portaldst
	del obj.anom_doorcount
	del obj.anom_bellnumber
	del obj.anom_objecttype
	del obj.anom_doorordered
	del obj.anom_doormovetime
	del obj.anom_targetentity
	del obj.anom_interactaction

	del obj.anom_soundmix
	del obj.anom_soundtype
	del obj.anom_soundpath
	del obj.anom_soundsize
	del obj.anom_soundsynth
	del obj.anom_soundreverb
	bpy.app.handlers.scene_update_post.clear()
	bpy.utils.unregister_module(__name__)

if __name__ == "__main__":
	register()
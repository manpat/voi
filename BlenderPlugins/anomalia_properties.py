#!BPY

import sys
import bpy
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
			ob.anom_newarea = ao.anom_newarea
			ob.anom_portaldst = ao.anom_portaldst
			ob.anom_objecttype = ao.anom_objecttype
			ob.anom_bellnumber = ao.anom_bellnumber
			ob.anom_targetentity = ao.anom_targetentity
			ob.anom_interactaction = ao.anom_interactaction
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
	obj.anom_doorordered = BoolProperty(name="Ordered Sequence")
	obj.anom_doorcount = IntProperty(name="Lock Count", min=1, max=30, default=1)

	sptypes = [
		('_', 'None', 'Not a sound', 0),
		('f', 'File', 'A sound loaded from a file', 1),
		('s', 'Synthesizer', 'A synthesized sound', 2),
	]

	obj.anom_soundtype = EnumProperty(items=sptypes, name="Speaker Type", default='_')
	obj.anom_soundpath = StringProperty(name="Sound Path")
	obj.anom_soundsynth = StringProperty(name="Synth Name")
	obj.anom_soundsize = FloatProperty(name="Sound Size", default=1.0, min=0.1, max=100.0)
	obj.anom_soundreverb = FloatProperty(name="Reverb Time", default=10000.0, min=10.0, max=20000.0)
	obj.anom_soundmix = FloatProperty(name="Reverb Mix", default=100.0, min=0.0, max=100.0)

	bpy.app.handlers.scene_update_post.append(poll_object_layer)
	bpy.utils.register_module(__name__)

def unregister():
	obj = bpy.types.Object
	del obj.anom_layer
	del obj.anom_hidden
	del obj.anom_newarea
	del obj.anom_portaldst
	del obj.anom_doorcount
	del obj.anom_bellnumber
	del obj.anom_objecttype
	del obj.anom_doorordered
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
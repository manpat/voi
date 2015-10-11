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

		otyp = context.active_object["anom_objecttype"]
		if(otyp == 1 or otyp == 2):
			row.prop(context.active_object, "anom_portaldst")

		if(otyp == 3):
			row = layout.row()
			row.prop(context.active_object, "anom_targetentity")
			row.prop(context.active_object, "anom_interactaction")

		if(otyp == 5):
			row = layout.row()
			row.prop(context.active_object, "anom_newarea")



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
	obj = bpy.types.Object
	obj.anom_layer = IntProperty(name="Layer",
		default=0, min=0, max=10, subtype='UNSIGNED',
		update=layer_update)

	items = [
		('l', 'Level Trigger', 'A trigger that loads a new area when entered', '', 5),
		('d', 'Door', 'An openable door', '', 4),
		('i', 'Interact', 'Is interactible. Triggers event(s) on interact', '', 3),
		('m', 'Mirror', 'Can reflect into another layer', '', 2),
		('p', 'Portal', 'Act\'s as a portal between layers', '', 1),
		('_', 'World', '', '', 0),
	]

	obj.anom_objecttype = EnumProperty(items=items,
		name="Object Type", default='_')

	obj.anom_portaldst = IntProperty(name="Destination Layer",
		min=0, max=10, default=1, subtype='UNSIGNED')

	obj.anom_targetentity = StringProperty(name="Target Entity")
	obj.anom_interactaction = StringProperty(name="Action")

	obj.anom_newarea = StringProperty(name="New Area Path")

	bpy.app.handlers.scene_update_post.append(poll_object_layer)
	bpy.utils.register_module(__name__)

def unregister():
	obj = bpy.types.Object
	del obj.anom_layer
	del obj.anom_newarea
	del obj.anom_portaldst
	del obj.anom_objecttype
	del obj.anom_targetentity
	del obj.anom_interactaction
	bpy.app.handlers.scene_update_post.clear()
	bpy.utils.unregister_module(__name__)

if __name__ == "__main__":
	register()
#!BPY

import sys
import bpy
from bpy.props import *

class UtilityPanel(bpy.types.Panel):
	bl_label = "Anomalia Properties"
	bl_idname = "OBJECT_PT_anomalia"

	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "object"

	def draw(self, context):
		layout = self.layout

		row = layout.row()
		# row.prop(context.active_object, "anom_portal")
		# row.prop(context.active_object, "anom_mirror")
		row.prop(context.active_object, "anom_objecttype")

		row = layout.row()
		row.prop(context.active_object, "anom_layer")

		otyp = context.active_object["anom_objecttype"]
		if(otyp == 'p' or otyp == 'm'):
			row.prop(context.active_object, "anom_portaldst")


bl_info = {
	"name": "Anomalia Properties",
	"author": "Patrick Monaghan",
	"description": "Adds properties specific to the development of Anomalia",
	"category": "Object",
	"version": (0, 0, 1),
	"blender": (2, 6, 9),
}

def register():
	obj = bpy.types.Object
	obj.anom_layer = IntProperty(name="Layer",
		default=0, min=0, max=10, subtype='UNSIGNED')

	items = [
		('d', 'Door', 'An openable door'),
		('i', 'Interact', 'Is interactible. Triggers event(s) on interact'),
		('m', 'Mirror', 'Can reflect into another layer'),
		('p', 'Portal', 'Act\'s as a portal between layers'),
		('_', 'World', ''),
	]

	obj.anom_objecttype = EnumProperty(items=items,
		name="Object Type", default='_')

	obj.anom_portaldst = IntProperty(name="Destination Layer",
		min=0, max=10, default=1, subtype='UNSIGNED')

	bpy.utils.register_module(__name__)

def unregister():
	obj = bpy.types.Object
	del obj.anom_layer
	del obj.anom_portaldst
	del obj.anom_objecttype
	bpy.utils.unregister_module(__name__)

if __name__ == "__main__":
	register()
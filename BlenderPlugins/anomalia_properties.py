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
		row.prop(context.active_object, "anom_portal")
		row.prop(context.active_object, "anom_mirror")

		row = layout.row()
		row.prop(context.active_object, "anom_layer")

		if(context.active_object["anom_portal"] or context.active_object["anom_mirror"]):
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

	obj.anom_portal = BoolProperty(name="Is Portal",
		default=False)

	obj.anom_mirror = BoolProperty(name="Is Mirror",
		default=False)

	obj.anom_portaldst = IntProperty(name="Destination Layer",
		min=0, max=10, default=1, subtype='UNSIGNED')

	bpy.utils.register_module(__name__)

def unregister():
	obj = bpy.types.Object
	del obj.anom_layer
	del obj.anom_portal
	del obj.anom_mirror
	del obj.anom_portaldst
	bpy.utils.unregister_module(__name__)

if __name__ == "__main__":
	register()
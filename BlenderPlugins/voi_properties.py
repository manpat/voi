
import bpy
import sys
import math
import mathutils
from bpy.props import *

bl_info = {
	"name": "Voi Properties",
	"author": "Patrick Monaghan",
	"description": "Adds properties specific to the development of Voi",
	"category": "Object",
	"version": (0, 0, 1),
	"blender": (2, 6, 9),
}

class ObjectPanel(bpy.types.Panel):
	bl_label = "Voi Properties"
	bl_idname = "OBJECT_PT_voi"

	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "object"

	def draw(self, context):
		layout = self.layout
		o = context.object

		layout.row().prop(o, "voi_entitytype")
		layout.row().prop(o, "voi_entityhidden")

def register():
	bpy.utils.register_class(ObjectPanel)

	bpy.types.Scene.voi_obtypes = [
		('g', "Geometry", "Shit that doesn't do much"),
		('p', "Portal", "Goes places"),
		('m', "Mirror", "Shiny"),
	]

	obj = bpy.types.Object
	obj.voi_entitytype = EnumProperty(items=bpy.types.Scene.voi_obtypes, name="Entity Type", default='g')
	obj.voi_entityhidden = BoolProperty(name="Hidden")


def unregister():
	bpy.utils.unregister_class(ObjectPanel)


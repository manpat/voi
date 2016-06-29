
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

voi_obtypes = [
	('g', "Geometry", "Shit that doesn't do much"),
	('p', "Portal", "Goes places"),
	('m', "Mirror", "Shiny"),
	('i', "Interactive", "Frob da thing"),
	('t', "Trigger", "Touch da thing"),
]

voi_coltypes = [
	('_', "None", "Itsa ghost"),
	('c', "Cube", "Itsa cube"),
	('y', "Cylinder", "Itsa cylinder"),
	('p', "Capsule", "Itsa capsule"),
	('h', "Convex Hull", "Clingwrap"),
	('m', "Mesh", "Itsa mesh. Kinda expensive"),
]

voi_scrtypes = [
	('c', "Script", "Shit that does shit"),
	('s', "Shader", "Looks good"),
	('d', "Synth Def", "Makes a bleep"),
]

class ObjectPanel(bpy.types.Panel):
	bl_label = "Voi Entity"
	bl_idname = "OBJECT_PT_voi"

	bl_space_type = "PROPERTIES"
	bl_region_type = "WINDOW"
	bl_context = "object"

	def draw_header(self, context):
		layout = self.layout
		o = context.object
		layout.prop(o, "voi_entitydoexport", text="")

	def draw(self, context):
		layout = self.layout
		o = context.object
		type = o.get("voi_entitytype", 0)

		layout.row().prop(o, "voi_entitytype")
		row = layout.row()
		if type == 0: # Only geometry can be static for now
			row.prop(o, "voi_entitystatic")
		row.prop(o, "voi_entityhidden")

		layout.row().prop(o, "voi_collidertype")
		layout.row().prop(o, "voi_entityupdatecb")

		if type == 3: # Interact
			layout.row().prop(o, "voi_entityfrobcb")

class ScriptPanel(bpy.types.Panel):
	bl_label = "Voi Script"
	bl_idname = "TEXT_PT_voi"

	bl_space_type = "TEXT_EDITOR"
	# bl_region_type = "WINDOW"
	bl_region_type = "UI"
	bl_context = "object"

	def draw(self, context):
		layout = self.layout
		o = context.object
		layout.row().prop(o, "voi_scripttype") # TODO This pls

def register():
	bpy.utils.register_class(ObjectPanel)
	bpy.utils.register_class(ScriptPanel)

	obj = bpy.types.Object
	obj.voi_entitytype = EnumProperty(items=voi_obtypes, name="Entity Type", default='g')
	obj.voi_collidertype = EnumProperty(items=voi_coltypes, name="Collider Type", default='_')
	obj.voi_entityhidden = BoolProperty(name="Hidden")
	obj.voi_entitystatic = BoolProperty(name="Static", default=True)
	obj.voi_entitydoexport = BoolProperty(name="Export", default=True)

	obj.voi_entityupdatecb = StringProperty(name="Update Callback", description="What to run when object is updated. file.lua:function_name")
	obj.voi_entityfrobcb = StringProperty(name="Frob Callback", description="What to run when object is frobbed. file.lua:function_name")

	obj = bpy.types.Text
	obj.voi_scripttype = EnumProperty(items=voi_scrtypes, name="Script Type", default='c')

def unregister():
	bpy.utils.unregister_class(ObjectPanel)
	bpy.utils.unregister_class(ScriptPanel)


#!BPY

import bpy
import sys
import math
import mathutils
from bpy.props import *
from bpy.app.handlers import persistent

# http://blender.stackexchange.com/questions/21463/draw-points-on-screen-with-bgl-python

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
		elif type == 4: # Trigger
			layout.row().prop(o, "voi_entityentercb")
			layout.row().prop(o, "voi_entityleavecb")

		layout.row().prop(o, "voi_entityignorefog")

def get_trigger_material():
	m = bpy.data.materials.get("_Trigger")
	if m == None:
		m = bpy.data.materials.new("_Trigger")
		m.use_transparency = True
		m.diffuse_color = (1,0,0)
		m.alpha = 0.6

	return m

def get_portal_material():
	m = bpy.data.materials.get("_Portal")
	if m == None:
		m = bpy.data.materials.new("_Portal")
		m.diffuse_color = (1,0,1)

	return m

tracked_portals = []
tracked_triggers = []

def update_voi_object(o):
	global tracked_portals, tracked_triggers

	if o == None: return
	if o.type != 'MESH': return
	etype = o.get("voi_entitytype", 0)

	if etype == 4:
		o.show_transparent = True
		o.show_wire = True
		o.active_material = get_trigger_material()
		o.draw_type = 'SOLID'
		tracked_triggers.append(o)
		if o in tracked_portals: tracked_portals.remove(o)

	elif etype == 1:
		o.show_wire = True
		o.active_material = get_portal_material()
		tracked_portals.append(o)
		if o in tracked_triggers: tracked_triggers.remove(o)

	elif o in tracked_portals: # Was a portal
		tracked_portals.remove(o)
		o.show_wire = False
		o.active_material = None

	elif o in tracked_triggers: # Was a trigger
		tracked_triggers.remove(o)
		o.draw_type = 'TEXTURED'
		o.show_wire = False
		o.show_transparent = False
		o.active_material = None

	else: return

	scene = bpy.context.scene
	if o.mode != 'EDIT':
		ao = scene.objects.active 
		scene.objects.active = o
		scene.objects.active = ao

@persistent
def voi_update_ui(x):
	scene = bpy.context.scene
	for o in scene.objects:
		update_voi_object(o)

prev_selected_obj = None
prev_selobj_type = 0

@persistent
def test_object_changes(x):
	global prev_selected_obj, prev_selobj_type

	scene = bpy.context.scene
	if scene.objects.active != prev_selected_obj:
		update_voi_object(prev_selected_obj)
		prev_selected_obj = scene.objects.active
		prev_selobj_type = prev_selected_obj.get("voi_entitytype", 0)
		return

	otype = prev_selected_obj.get("voi_entitytype", 0)
	if prev_selobj_type != otype:
		update_voi_object(prev_selected_obj)
		prev_selobj_type = otype
		return

# This is to avoid dangling references
@persistent
def voi_load(x):
	global prev_selected_obj, prev_selobj_type
	global tracked_portals, tracked_triggers
	prev_selected_obj = None
	prev_selobj_type = 0
	tracked_portals = []
	tracked_triggers = []
	voi_update_ui(x)

def register():
	bpy.utils.register_class(ObjectPanel)

	obj = bpy.types.Object
	obj.voi_entitytype = EnumProperty(items=voi_obtypes, name="Entity Type", default='g')
	obj.voi_collidertype = EnumProperty(items=voi_coltypes, name="Collider Type", default='_')
	obj.voi_entityhidden = BoolProperty(name="Hidden")
	obj.voi_entitystatic = BoolProperty(name="Static", default=True)
	obj.voi_entitydoexport = BoolProperty(name="Export", default=True)
	obj.voi_entityignorefog = BoolProperty(name="Ignore Fog", default=False)

	obj.voi_entityupdatecb = StringProperty(name="Update Callback", description="What to run when object is updated.")
	obj.voi_entityfrobcb = StringProperty(name="Frob Callback", description="What to run when object is frobbed.")
	obj.voi_entityentercb = StringProperty(name="Enter Callback", description="What to run when trigger is entered.")
	obj.voi_entityleavecb = StringProperty(name="Leave Callback", description="What to run when trigger is left.")

	bpy.app.handlers.save_post.append(voi_update_ui)
	bpy.app.handlers.load_post.append(voi_load)
	bpy.app.handlers.scene_update_post.append(test_object_changes)

def unregister():
	bpy.utils.unregister_class(ObjectPanel)
	bpy.app.handlers.save_post.remove(voi_update_ui)
	bpy.app.handlers.load_post.remove(voi_load)
	bpy.app.handlers.scene_update_post.remove(test_object_changes)
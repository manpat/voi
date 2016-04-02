#!BPY

bl_info = {
	"name": "Voi Scene Exporter",
	"author": "Patrick Monaghan",
	"description": "Exports scenes in a format that voi can eat",
	"category": "Export",
	"version": (0, 0, 1),
	"blender": (2, 6, 9),
}

import struct
import bpy
from bpy import context

# https://docs.python.org/2/library/struct.html

class ExportVoiScene(bpy.types.Operator):
	"""Voi scene exporter"""
	bl_idname = "export.voi_scene"
	bl_label = "Export Voi Scene"

	filepath = bpy.props.StringProperty(subtype="FILE_PATH")

	# @classmethod
	# def poll(cls, context):
	# 	return context.object is not None

	def execute(self, context):
		fname = self.filepath
		if(fname[:-4].lower() != ".voi"):
			fname += ".voi"

		with open(fname, 'wb') as out:
			out.write(b"VOI") # Magic
			out.write(struct.pack('=B', 1)) # Version

			scene = bpy.data.scenes[0]
			for obj in scene.objects:
				if(obj.type == 'MESH'):
					out.write(struct.pack('=I', len(obj.data.vertices)))
					for v in obj.data.vertices:
						out.write(struct.pack('=3f', *v.co[:]))

					out.write(struct.pack('=I', len(obj.data.polygons)))
					for f in obj.data.polygons:
						out.write(struct.pack('=%dI' % len(f.vertices), *f.vertices[:]))


		return {'FINISHED'}

	def invoke(self, context, event):
		context.window_manager.fileselect_add(self)
		return {'RUNNING_MODAL'}


# Only needed if you want to add into a dynamic menu
def menu_func(self, context):
	self.layout.operator_context = 'INVOKE_DEFAULT'
	self.layout.operator(ExportVoiScene.bl_idname, text="Voi Scene (.voi)")

# Register and add to the file selector
def register():
	bpy.utils.register_class(ExportVoiScene)
	bpy.types.INFO_MT_file_export.append(menu_func)

def unregister():
	bpy.utils.unregister_class(ExportVoiScene)
	bpy.types.INFO_MT_file_export.remove(menu_func)

# # test call
# bpy.ops.export.some_data('INVOKE_DEFAULT')
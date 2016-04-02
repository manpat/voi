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
		if(fname == ""):
			self.report({'ERROR'}, "Empty filepath!")
			return {'FINISHED'}

		if(fname[-4:].lower() != ".voi"):
			fname += ".voi"

		self.compileMaterials()
		self.compileMeshes()
		self.compileEntities()

		print(self.materials, "\n")
		print(self.meshes, "\n")
		print(self.entities, "\n")

		with open(fname, 'wb') as out:
			out.write(b"VOI") # Magic
			out.write(struct.pack('=B', 1)) # Version

			out.write(struct.pack('=H', len(self.meshes)))
			for m in self.meshes:
				numVertices = m['numVertices']

				out.write(b"MESH")
				out.write(struct.pack('=I', numVertices))
				for v in m['vertices']:
					out.write(struct.pack('=f', v))

				packStr = '=I'
				if numVertices < 256:
					packStr = '=B'
				elif numVertices < 65536:
					packStr = '=H'
				
				out.write(struct.pack('=I', m['numTriangles']))
				for t in m['triangles']:
					out.write(struct.pack(packStr, t))

				for mid in m['materialIDs']:
					out.write(struct.pack('=B', mid))

			out.write(struct.pack('=B', len(self.materials)))
			for m in self.materials:
				out.write(b"MATL")
				out.write(struct.pack('=B', len(m['name'])))
				out.write(bytes(m['name'], 'utf-8'))
				out.write(struct.pack('=fff', *m['color']))

			out.write(struct.pack('=H', len(self.entities)))
			for e in self.entities:
				out.write(b"ENTY")
				out.write(struct.pack('=B', len(e['name'])))
				out.write(bytes(e['name'], 'utf-8'))

				out.write(struct.pack('=fff', *e['position']))
				out.write(struct.pack('=fff', *e['rotation']))
				out.write(struct.pack('=H', e['parentID']))
				out.write(struct.pack('=H', e['meshID']))
				out.write(struct.pack('=H', e['scriptID']))
				out.write(struct.pack('=B', e['entityType']))
				out.write(struct.pack('=B', e['colliderType']))
				# TODO: collider data

			out.write(struct.pack('=H', len(bpy.data.texts)))
			for s in bpy.data.texts:
				out.write(b"CODE")
				out.write(struct.pack('=B', len(s.name)))
				out.write(bytes(s.name, 'utf-8'))

				str = s.as_string()
				out.write(struct.pack('=I', len(str)))
				out.write(bytes(str, 'utf-8'))

		return {'FINISHED'}

	def compileMaterials(self):
		self.materials = []
		self.materialIDs = {}

		for m in bpy.data.materials:
			mat = {}
			mat['name'] = m.name
			mat['color'] = m.diffuse_color

			self.materials.append(mat)
			self.materialIDs[m.name] = len(self.materials)

	def compileMeshes(self):
		# This should compile static meshes on a layer into a single mesh
		self.meshes = []
		self.meshIDs = {}

		scene = bpy.data.scenes[0]
		for obj in scene.objects:
			if obj.type == 'MESH':
				odata = obj.data
				if odata.name in self.meshIDs: continue

				vs = []
				ts = []
				ms = []
				numTriangles = 0

				for v in odata.vertices:
					vs.extend(v.co[:])

				for p in odata.polygons:
					midx = p.material_index
					mname = odata.materials[midx].name
					mid = self.materialIDs.get(mname, 0)

					if len(p.vertices) == 3:
						ts.extend(p.vertices[:])
						ms.append(mid)
						numTriangles += 1
					else: # quad
						tvs = p.vertices
						ts.extend([tvs[0], tvs[1], tvs[2]])
						ts.extend([tvs[0], tvs[2], tvs[3]])
						ms.extend([mid, mid])
						numTriangles += 2

				mesh = {
					'numVertices': len(odata.vertices),
					'vertices': vs,
					'numTriangles': numTriangles,
					'triangles': ts,
					'materialIDs': ms,
				}

				self.meshes.append(mesh)
				self.meshIDs[odata.name] = len(self.meshes) # ids start at 1
			else:
				print(obj.type)

	def compileEntities(self):
		self.entities = []

		scene = bpy.data.scenes[0]
		for obj in scene.objects:
			mid = self.meshIDs.get(obj.data.name, 0)
			pos = obj.location.xyz[:]
			rot = obj.rotation_euler[:]

			self.entities.append({
				'name': obj.name,

				'position': pos,
				'rotation': rot,

				'parentID': 0, # TODO
				'meshID': mid,
				'scriptID': 0, # TODO

				'entityType': 0, # TODO
				'colliderType': 0, # TODO
			})

	def invoke(self, context, event):
		context.window_manager.fileselect_add(self)
		return {'RUNNING_MODAL'}


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

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

def swapCoords(co):
	return [co.x, co.z, -co.y];

class ExportVoiScene(bpy.types.Operator):
	"""Voi scene exporter"""
	bl_idname = "export.voi_scene"
	bl_label = "Export Voi Scene"

	filepath = bpy.props.StringProperty(subtype="FILE_PATH")

	# @classmethod
	# def poll(cls, context):
	# 	return context.object is not None

	def execute(self, context):
		debugRun = False

		fname = self.filepath
		if(fname == ""):
			debugRun = True

		if(fname[-4:].lower() != ".voi"):
			fname += ".voi"

		self.compileMaterials()
		self.compileMeshes()
		self.compileEntities()

		print(bpy.types.Scene.voi_obtypes)

		# print(self.materials, "\n")
		# print(self.meshes, "\n")
		# print(self.entities, "\n")
		if debugRun:
			self.report({'ERROR'}, "Empty filepath!")
			return {'FINISHED'}


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
				out.write(struct.pack('=B', e['layer']))
				out.write(struct.pack('=I', e['flags']))
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

		defaultMat = {'name': 'DEFAULT', 'color': [1,0,1]}
		self.materials.append(defaultMat)

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
					vs.extend(swapCoords(v.co))

				for p in odata.polygons:
					midx = p.material_index
					mid = 0

					if midx < len(odata.materials):
						mat = odata.materials[midx]
						mid = self.materialIDs.get(mat.name, 0)

					if len(p.vertices) == 3:
						ts.extend(p.vertices[:])
						ms.append(mid)
						numTriangles += 1
					elif len(p.vertices) >= 4:
						tvs = p.vertices
						for i in range(1, len(tvs)-1):
							ts.extend([tvs[0], tvs[i], tvs[i+1]])

						ms.extend([mid] * (len(tvs)-2))
						numTriangles += len(tvs)-2
					# else:
						# ERROR

				mesh = {
					'numVertices': len(odata.vertices),
					'vertices': vs,
					'numTriangles': numTriangles,
					'triangles': ts,
					'materialIDs': ms,
				}

				self.meshes.append(mesh)
				self.meshIDs[odata.name] = len(self.meshes) # ids start at 1
			# else:
			# 	print("Unknown type: " + obj.type)

	def compileEntities(self):
		self.entities = []

		obtypes = bpy.types.Scene.voi_obtypes
		scene = bpy.data.scenes[0]
		for obj in scene.objects:
			mid = self.meshIDs.get(obj.data.name, 0)
			pos = swapCoords(obj.location.xyz)
			rot = swapCoords(obj.rotation_euler)

			layer = next(i for i,v in enumerate(obj.layers) if v)
			type = obj.get("voi_entitytype", 0)
			flags = 0

			if obj.get("voi_entityhidden", False):
				flags |= 1<<0

			print(type)

			self.entities.append({
				'name': obj.name,

				'position': pos,
				'rotation': rot,
				'layer': layer,

				'flags': flags,

				'parentID': 0, # TODO
				'meshID': mid,
				'scriptID': 0, # TODO

				'entityType': type,
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

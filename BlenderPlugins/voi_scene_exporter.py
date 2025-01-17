#!BPY

bl_info = {
	"name": "Voi Scene Exporter",
	"author": "Patrick Monaghan",
	"description": "Exports scenes in a format that voi can eat",
	"category": "Export",
	"version": (0, 0, 1),
	"blender": (2, 6, 9),
}

import mathutils
import struct
import bpy
import bmesh
from bpy import context

# https://docs.python.org/2/library/struct.html

def swapCoords(co):
	return [co.x, co.z, -co.y]

class ExportVoiScene(bpy.types.Operator):
	"""Voi scene exporter"""
	bl_idname = "export.voi_scene"
	bl_label = "Export Voi Scene"

	filepath = bpy.props.StringProperty(subtype="FILE_PATH")

	def execute(self, context):
		debugRun = False

		fname = self.filepath
		if(fname == ""):
			debugRun = True

		if(fname[-4:].lower() != ".voi"):
			fname += ".voi"
			self.filepath += ".voi"

		self.compileMaterials()
		self.compileMeshes()
		self.compileEntities()

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

			out.write(b"MATL")
			out.write(struct.pack('=B', len(self.materials)))
			for m in self.materials:
				out.write(struct.pack('=fff', *m['color']))

			out.write(struct.pack('=H', len(self.entities)))
			for e in self.entities:
				out.write(b"ENTY")
				out.write(struct.pack('=B', len(e['name'])))
				out.write(bytes(e['name'], 'utf-8'))

				type = e['entityType']
				col = e['colliderType']

				out.write(struct.pack('=fff', *e['position']))
				out.write(struct.pack('=fff', *e['rotation']))
				out.write(struct.pack('=fff', *e['scale']))
				out.write(struct.pack('=I', e['layers']))
				out.write(struct.pack('=I', e['flags']))
				out.write(struct.pack('=H', e['parentID']))
				out.write(struct.pack('=H', e['meshID']))
				out.write(struct.pack('=B', type))
				out.write(struct.pack('=B', col))
				# TODO: collider data

				icbstr = bytes(e['init'], 'utf-8')
				out.write(struct.pack('=B', len(icbstr)))
				out.write(icbstr)

				ucbstr = bytes(e['update'], 'utf-8')
				out.write(struct.pack('=B', len(ucbstr)))
				out.write(ucbstr)

				if type in [1, 2]: # Portal, Mirror
					out.write(struct.pack('=H', 12)) # Size
					out.write(struct.pack('=fff', *e['planeNormal']))
				elif type == 3:
					ostr = bytes(e['frobcb'], 'utf-8')
					out.write(struct.pack('=H', len(ostr)+1)) # Size
					out.write(struct.pack('=B', len(ostr))) # This might be overkill but whatever
					out.write(ostr)
				elif type == 4:
					enterstr = bytes(e['entercb'], 'utf-8')
					leavestr = bytes(e['leavecb'], 'utf-8')
					out.write(struct.pack('=H', len(enterstr)+len(leavestr)+2)) # Size
					out.write(struct.pack('=B', len(enterstr)))
					out.write(enterstr)
					out.write(struct.pack('=B', len(leavestr)))
					out.write(leavestr)
				else:
					out.write(struct.pack('=H', 0))

		return {'FINISHED'}

	def compileMaterials(self):
		self.materials = []
		self.materialIDs = {}

		defaultMat = {'name': 'DEFAULT', 'color': [1,0,1]}
		self.materials.append(defaultMat)

		for m in bpy.data.materials:
			if m.name[0] == '_':
				continue

			mat = {}
			mat['name'] = m.name
			mat['color'] = m.diffuse_color

			self.materials.append(mat)
			self.materialIDs[m.name] = len(self.materials)

	def compileMeshes(self):
		# This should compile static meshes on a layer into a single mesh
		self.meshes = []
		self.meshIDs = {}

		scene = bpy.context.scene
		for obj in scene.objects:
			if obj.type == 'MESH':
				odata = obj.data
				if odata.name in self.meshIDs: continue

				bm = bmesh.new()
				bm.from_object(obj, scene, deform=True)
				bmesh.ops.triangulate(bm, faces=bm.faces)

				bm.verts.ensure_lookup_table()
				bm.faces.ensure_lookup_table()

				vs = []
				ts = []
				ms = []
				numTriangles = 0

				for v in bm.verts:
					vs.extend(swapCoords(v.co))

				for p in bm.faces:
					midx = p.material_index
					mid = 0

					if midx < len(odata.materials):
						mat = odata.materials[midx]
						if mat:
							mid = self.materialIDs.get(mat.name, 0)

					# Assumes there are always 3 verts
					for v in p.verts:
						ts.append(v.index)

					ms.append(mid)
					numTriangles += 1

				mesh = {
					'numVertices': len(bm.verts),
					'vertices': vs,
					'numTriangles': numTriangles,
					'triangles': ts,
					'materialIDs': ms,
				}

				bm.free()
				del bm

				self.meshes.append(mesh)
				self.meshIDs[odata.name] = len(self.meshes) # ids start at 1

	def compileEntities(self):
		self.entities = []

		scene = bpy.context.scene
		for obj in scene.objects:
			if not obj.get("voi_entitydoexport", True):
				continue

			mid = 0
			if obj.data:
				mid = self.meshIDs.get(obj.data.name, 0)
			
			type = obj.get("voi_entitytype", 0)
			col = obj.get("voi_collidertype", 0)

			flags = 0
			if obj.get("voi_entityhidden", False):
				flags |= 1<<0

			if obj.get("voi_entitystatic", True) and type==0:
				flags |= 1<<1

			if obj.get("voi_entityignorefog", False):
				flags |= 1<<2

			if obj.get("voi_entitydoublesided", False):
				flags |= 1<<3

			scale = obj.scale
			scale = [scale.x, scale.z, scale.y]

			data = {
				'name': obj.name,

				'position': swapCoords(obj.location.xyz),
				'rotation': swapCoords(obj.rotation_euler),
				'scale': scale,
				'layers': sum(1<<i for i,v in enumerate(obj.layers) if v),

				'flags': flags,

				'parentID': 0, # TODO
				'meshID': mid,

				'entityType': type,
				'colliderType': col,

				'init': obj.get("voi_entityinitcb", ""),
				'update': obj.get("voi_entityupdatecb", ""),
			}

			if type in [1, 2]:
				accum = mathutils.Vector((0,0,0))
				for p in obj.data.polygons:
					accum += p.normal
				accum /= len(obj.data.polygons)

				data['planeNormal'] = swapCoords(accum)
			elif type == 3:
				data['frobcb'] = obj.get("voi_entityfrobcb", "")
			elif type == 4:
				data['entercb'] = obj.get("voi_entityentercb", "")
				data['leavecb'] = obj.get("voi_entityleavecb", "")

			self.entities.append(data)

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

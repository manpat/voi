#include "meshinfo.h"
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreMesh.h>

std::vector<vec3> GetOgreSubMeshVertices(Ogre::SubMesh* sm){
	auto mesh = sm->parent;
	auto vertexData = sm->useSharedVertices? 
		mesh->sharedVertexData : sm->vertexData;

	auto indexData = sm->indexData;
	auto ibuf = indexData->indexBuffer;

	bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

	u32* idx32 = static_cast<u32*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
	u16* idx16 = reinterpret_cast<u16*>(idx32);

	auto posElem = vertexData->vertexDeclaration
		->findElementBySemantic(Ogre::VES_POSITION);

	auto vbuf = vertexData->vertexBufferBinding
		->getBuffer(posElem->getSource());

	u8* vertex = static_cast<u8*>(
		vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
	
	std::vector<vec3> vertices;
	vertices.reserve(vertexData->vertexCount);

	f32* vecPtr = nullptr;
	auto idxStart = indexData->indexStart;
	for(u64 i = idxStart; i < indexData->indexCount+idxStart; i++){
		auto idx = (use32bitindexes)?idx32[i]:(u32)idx16[i];

		posElem->baseVertexPointerToElement(vertex+idx*vbuf->getVertexSize(), &vecPtr);
		vertices.push_back(vec3(vecPtr[0], vecPtr[1], vecPtr[2]));
	}

	vbuf->unlock();
	ibuf->unlock();

	return vertices;
}

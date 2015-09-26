#include "entitymotionstate.h"
#include "entity.h"

#include <OGRE/OgreSceneNode.h>

EntityMotionState::EntityMotionState(Entity* e){
	if(!e) throw "Tried to create EntityMotionState with null entity";

	sceneNode = e->ogreSceneNode;
}

void EntityMotionState::getWorldTransform(btTransform& worldTrans) const {
	// TODO: Does this actually have to be a saved initial state?
	//	is this ok?
	auto pos = sceneNode->_getDerivedPosition();
	auto ori = sceneNode->_getDerivedOrientation();
	worldTrans.setIdentity();
	worldTrans.setOrigin({pos.x, pos.y, pos.z});
	worldTrans.setRotation({ori.x, ori.y, ori.z, ori.w});
}

void EntityMotionState::setWorldTransform(const btTransform& newTrans) {
	auto ori = newTrans.getRotation();
	auto pos = newTrans.getOrigin();
	sceneNode->_setDerivedOrientation({ori.w(), ori.x(), ori.y(), ori.z()});
	sceneNode->_setDerivedPosition({pos.x(), pos.y(), pos.z()});
}

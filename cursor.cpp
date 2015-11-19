#include "app.h"
#include "common.h"
#include "cursor.h"
#include "layerrenderingmanager.h"

#include <OGRE/OgreMaterial.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreTechnique.h>
#include <OGRE/OgreRectangle2D.h>
#include <OGRE/OgreMaterialManager.h>

Cursor::Cursor() {
	mat = Ogre::MaterialManager::getSingleton().create("Cursor", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	auto matPass = mat->getTechnique(0)->getPass(0);

	matPass->createTextureUnitState("cursor.png");
	matPass->setDepthWriteEnabled(false);
	matPass->setDepthCheckEnabled(false);
	matPass->setLightingEnabled(false);
	matPass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);

	if (!matPass->getTextureUnitState(0)->_getTexturePtr().isNull())
		diameter = matPass->getTextureUnitState(0)->_getTexturePtr()->getWidth();
	else
		diameter = 32;

	cursor = new Ogre::Rectangle2D(true);

	auto left = -((WIDTH / 2.0f) - ((WIDTH / 2.0f) - (diameter / 2.0f))) / WIDTH;
	auto top = ((HEIGHT / 2.0f) - ((HEIGHT / 2.0f) - (diameter / 2.0f))) / HEIGHT;
	auto right = -left;
	auto bottom = -top;
	cursor->setCorners(left, top, right, bottom);

	cursor->setMaterial("Cursor");

	cursor->setRenderQueueGroup(RENDER_QUEUE_UI);

	Ogre::AxisAlignedBox aab;
	aab.setInfinite();
	cursor->setBoundingBox(aab);

	node = App::GetSingleton()->rootNode->createChildSceneNode("Cursor");
	node->attachObject(cursor);
}

Cursor::~Cursor() {
	cursor = nullptr;
}

void Cursor::Enable() {
	cursor->setVisible(true);
}

void Cursor::Disable() {
	cursor->setVisible(false);
}
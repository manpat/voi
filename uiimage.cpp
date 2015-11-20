#include "uiimage.h"

#include "app.h"
#include "layerrenderingmanager.h"

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreTechnique.h>
#include <OGRE/OgreRectangle2D.h>
#include <OGRE/OgreMaterialManager.h>

UiImage::UiImage() {

}

UiImage::~UiImage() {
	rect = nullptr;
	matPass = nullptr;
}

void UiImage::Init() {
	mat = Ogre::MaterialManager::getSingleton().create(GetName(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	matPass = mat->getTechnique(0)->getPass(0);

	matPass->setDepthWriteEnabled(false);
	matPass->setDepthCheckEnabled(false);
	matPass->setLightingEnabled(false);
	matPass->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
	//matPass->setCullingMode(Ogre::CULL_NONE);

	rect = new Ogre::Rectangle2D(true);

	rect->setRenderQueueGroup(RENDER_QUEUE_UI);
	rect->setCorners(-0.1f, 0.1f, 0.1f, -0.1f);
	rect->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);

	node->attachObject(rect);
}

void UiImage::SetImage(const std::string& filename, bool resize) {
	matPass->createTextureUnitState(filename);

	if (resize)
		ResizeObjectToImage();

	rect->setMaterial(GetName());
}

void UiImage::ResizeObjectToImage() {
	if (!matPass->getTextureUnitState(0)->_getTexturePtr().isNull()) {
		size = vec2((f32)matPass->getTextureUnitState(0)->_getTexturePtr()->getWidth(), (f32)matPass->getTextureUnitState(0)->_getTexturePtr()->getHeight());
	}
	else {
		std::cout << "Can't get texture pointer to UI object " << GetName() << ", Defaulting to 128x128\n";
		size = vec2(128, 128);
	}

	auto left = -((WIDTH / 2.0f) - ((WIDTH / 2.0f) - (size.x / 2.0f))) / WIDTH;
	auto top = ((HEIGHT / 2.0f) - ((HEIGHT / 2.0f) - (size.y / 2.0f))) / HEIGHT;
	auto right = -left;
	auto bottom = -top;
	rect->setCorners(left, top, right, bottom);
}

void UiImage::SetVisible(bool visible) {
	rect->setVisible(visible);
}
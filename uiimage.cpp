#include "uiimage.h"

#include "app.h"
#include "layerrenderingmanager.h"

#include <OGRE/OgreColourValue.h>
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

void UiImage::SetColour(f32 r, f32 g, f32 b, f32 a) {
	matPass->setDiffuse(Ogre::ColourValue(r, g, b, a));
}

void UiImage::SetImage(const std::string& filename, bool resize) {
	matPass->createTextureUnitState(filename);

	if (resize)
		ResizeObjectToImage();

	rect->setMaterial(GetName());
}

void UiImage::SetPosition(f32 x, f32 y) {
	position.x = x;
	position.y = y;

	CalculateTransform();
}

void UiImage::SetSize(u32 w, u32 h) {
	size.x = w;
	size.y = h;

	CalculateTransform();
}

void UiImage::SetVisible(bool visible) {
	rect->setVisible(visible);
}

Ogre::AxisAlignedBox UiImage::GetAABB() {
	return rect->getBoundingBox();
}

void UiImage::ResizeObjectToImage() {
	if (!matPass->getTextureUnitState(0)->_getTexturePtr().isNull()) {
		size = vec2((f32)matPass->getTextureUnitState(0)->_getTexturePtr()->getWidth(), (f32)matPass->getTextureUnitState(0)->_getTexturePtr()->getHeight());
	}
	else {
		std::cout << "Can't get texture pointer to UI object " << GetName() << ", Defaulting to 128x128\n";
		size = vec2(128, 128);
	}
	
	CalculateTransform();
}

void UiImage::CalculateTransform() {
	vec2 screenOffset, screenSize;

	switch (alignment) {
	case Alignment::TopLeft:
		screenOffset.x = 0.0f;
		screenOffset.y = size.y / HEIGHT;
		break;
	case Alignment::TopCenter:
		screenOffset.x = (size.x / WIDTH) / 2;
		screenOffset.y = size.y / HEIGHT;
		break;
	case Alignment::TopRight:
		screenOffset.x = size.x / WIDTH;
		screenOffset.y = size.y / HEIGHT;
		break;
	case Alignment::CenterLeft:
		screenOffset.x = 0.0f;
		screenOffset.y = (size.y / HEIGHT) / 2;
		break;
	case Alignment::Center:
		screenOffset.x = (size.x / WIDTH) / 2;
		screenOffset.y = (size.y / HEIGHT) / 2;
		break;
	case Alignment::CenterRight:
		screenOffset.x = size.x / WIDTH;
		screenOffset.y = (size.y / HEIGHT) / 2;
		break;
	case Alignment::BottomLeft:
		screenOffset.x = 0.0f;
		screenOffset.y = 0.0f;
		break;
	case Alignment::BottomCenter:
		screenOffset.x = (size.x / WIDTH) / 2;
		screenOffset.y = 0.0f;
		break;
	case Alignment::BottomRight:
		screenOffset.x = size.x / WIDTH;
		screenOffset.y = 0.0f;
		break;
	default:
		screenOffset.x = (size.x / WIDTH) / 2;
		screenOffset.y = (size.y / HEIGHT) / 2;
		break;
	}

	screenSize.x = size.x / WIDTH;
	screenSize.y = size.y / HEIGHT;

	auto left = position.x - screenOffset.x;
	auto bottom = position.y - screenOffset.y;
	auto top = bottom + screenSize.y;
	auto right = left + screenSize.x;

	rect->setCorners(left, top, right, bottom);
}
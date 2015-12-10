#include "uiimage.h"

#include "app.h"
#include "uimanager.h"
#include "layerrenderingmanager.h"

#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreTechnique.h>
#include <OGRE/OgreColourValue.h>
#include <OGRE/OgreRectangle2D.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreMaterialManager.h>

UiImage::UiImage() {

}

UiImage::~UiImage() {
	if (node) {
		App::GetSingleton()->sceneManager->destroySceneNode(node);
	}
	delete rect;
	rect = nullptr;
	matPass = nullptr;
}

void UiImage::Init() {
	mat = Ogre::MaterialManager::getSingleton().create(GetName(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	matPass = mat->getTechnique(0)->getPass(0);

	mat->setDepthWriteEnabled(false);
	mat->setDepthCheckEnabled(false);
	mat->setLightingEnabled(false);
	
	mat->setSceneBlending(Ogre::SceneBlendType::SBT_TRANSPARENT_ALPHA);

	rect = new Ogre::Rectangle2D(true);

	rect->setRenderQueueGroup(RENDER_QUEUE_UI);
	rect->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
	rect->setCorners(-0.1f, 0.1f, 0.1f, -0.1f);

	node->attachObject(rect);
}

void UiImage::FixedSize(bool f) {
	fixedSize = f;
	CalculateTransform();
}

void UiImage::MaintainRatio(bool m) {
	maintainRatio = m;
	CalculateTransform();
}

void UiImage::SetColour(f32 r, f32 g, f32 b, f32 a) {
	matPass->setColourWriteEnabled(true);
	matPass->setDiffuse(Ogre::ColourValue(r, g, b, a));
	matPass->setSelfIllumination(Ogre::ColourValue(1.0f, 1.0f, 1.0f, 1.0f));
	mat->setReceiveShadows(false);
	matPass->setLightingEnabled(true);
	matPass->setSeparateSceneBlending(Ogre::SceneBlendType::SBT_TRANSPARENT_ALPHA, Ogre::SceneBlendType::SBT_ADD);
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
	size.x = (f32)w;
	size.y = (f32)h;

	CalculateTransform();
}

void UiImage::SetVisible(bool visible) {
	rect->setVisible(visible);
}

Ogre::AxisAlignedBox UiImage::GetAABB() const {
	return rect->getBoundingBox();
}

void UiImage::ResizeObjectToImage() {
	if (!matPass->getTextureUnitState(0)->_getTexturePtr().isNull()) {
		size = vec2((f32)matPass->getTextureUnitState(0)->_getTexturePtr()->getWidth(), (f32)matPass->getTextureUnitState(0)->_getTexturePtr()->getHeight());
		std::cout << GetName() << " : " << size << "\n";
	}
	else {
		std::cout << "Can't get texture pointer to UI object " << GetName() << ", Defaulting to 128x128\n";
		size = vec2(128, 128);
	}
	
	CalculateTransform();
}

void UiImage::CalculateTransform() {
	vec2 screenOffset, screenSize;

	auto app = App::GetSingleton();

	s32 width, height;
	
	if (fixedSize) {
		width = app->GetWindowWidth();
		height = app->GetWindowHeight();
	}
	else {
		width = app->uiManager->GetUiWidth();
		height = app->uiManager->GetUiHeight();
	}

	if (maintainRatio) {
		screenSize.y = size.y / height;
		// Calculate width based on the height, adjusted to screen ratio, then adjusted to texture ratio
		screenSize.x = (screenSize.y * ((f32)app->GetWindowHeight() / app->GetWindowWidth())) * (size.x / size.y);
	}
	else {
		screenSize.x = size.x / width;
		screenSize.y = size.y / height;
	}

	switch (alignment) {
	case Alignment::TopLeft:
		screenOffset.x = 0.0f;
		screenOffset.y = screenSize.y;
		break;
	case Alignment::TopCenter:
		screenOffset.x = screenSize.x / 2;
		screenOffset.y = screenSize.y;
		break;
	case Alignment::TopRight:
		screenOffset.x = screenSize.x;
		screenOffset.y = screenSize.y;
		break;
	case Alignment::CenterLeft:
		screenOffset.x = 0.0f;
		screenOffset.y = screenSize.y / 2;
		break;
	case Alignment::Center:
		screenOffset.x = screenSize.x / 2;
		screenOffset.y = screenSize.y / 2;
		break;
	case Alignment::CenterRight:
		screenOffset.x = screenSize.x;
		screenOffset.y = screenSize.y / 2;
		break;
	case Alignment::BottomLeft:
		screenOffset.x = 0.0f;
		screenOffset.y = 0.0f;
		break;
	case Alignment::BottomCenter:
		screenOffset.x = screenSize.x / 2;
		screenOffset.y = 0.0f;
		break;
	case Alignment::BottomRight:
		screenOffset.x = screenSize.x;
		screenOffset.y = 0.0f;
		break;
	default:
		screenOffset.x = screenSize.x / 2;
		screenOffset.y = screenSize.y / 2;
		break;
	}

	auto left = position.x - screenOffset.x * 2;
	auto bottom = position.y - screenOffset.y * 2;
	auto top = bottom + screenSize.y * 2;
	auto right = left + screenSize.x * 2;

	rect->setCorners(left, top, right, bottom);
}
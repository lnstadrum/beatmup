#include "../basic_types.h"
#include "../scene/scene.h"
#include "../exception.h"
#include "../color/colors.h"
#include <map>
#include <vector>
#include <cmath>

using namespace Beatmup;
using namespace std;


string Scene::SceneIntegrityError::getSceneLog(const Scene& scene, const string prefix, int recursionLeft) const {
	string log = prefix + "Scene has " + std::to_string(scene.getLayerCount()) + " layers\n";
	const int N = scene.getLayerCount();
	for (int i = 0; i < N; i++) {
		Layer& layer = scene.getLayer(i);
		log += prefix + "\t" + layer.getName() + " [";
		switch (layer.getType()) {
			case Layer::Type::BitmapLayer: {
				log += "bitmap";
				BitmapLayer& casted = layer.castTo<BitmapLayer&>();
				if (!casted.getBitmap())
					log += ", no bitmap";
				else
					log += ", " + casted.getBitmap()->toString();
			}
			break;

			case Layer::Type::MaskedBitmapLayer: {
				log += "masked bitmap";
				MaskedBitmapLayer& casted = layer.castTo<MaskedBitmapLayer&>();
				if (!casted.getBitmap())
					log += ", no bitmap";
				else
					log += ", bitmap " + casted.getBitmap()->toString();
				if (!casted.getMask())
					log += ", no mask";
				else
					log += ", mask " + casted.getMask()->toString();
			}
			break;

			case Layer::Type::ShapedBitmapLayer:{
				log += "bitmap";
				BitmapLayer& casted = layer.castTo<BitmapLayer&>();
				if (!casted.getBitmap())
					log += ", no content";
				else
					log += ", " + casted.getBitmap()->toString();
			}
			break;

			case Layer::Type::ShadedBitmapLayer:
				log += "shader";
				break;

			case Layer::Type::SceneLayer:
				log += "scene";
				break;
		}
		if (layer.isPhantom())
			log += ", phantom";
		log += "]\n";
		if (layer.getType() == Layer::Type::SceneLayer) {
			if (recursionLeft > 0)
				log += getSceneLog(scene, prefix + "\t", recursionLeft - 1);
		}
	}
	return log;
}


Scene::SceneIntegrityError::SceneIntegrityError(const std::string reason, const Scene& scene) :
	Exception( (std::string(reason) + "\n" + getSceneLog(scene, "")).c_str() )
{}



std::string generateUniqueLayerName(const Scene& scene, const char* prefix = "") {
	int n = 1;
	std::string candidate;
	while (scene.getLayer((candidate = prefix + (" #" + to_string(n))).c_str()))
		n++;
	return candidate;
}


Scene::Scene() {}


Scene::~Scene() {
	for (auto it : layers)
		delete it;
}


Scene::BitmapLayer& Scene::newBitmapLayer(const char* name) {
	return newLayer<BitmapLayer>(name);
}


Scene::BitmapLayer& Scene::newBitmapLayer() {
	return newLayer<BitmapLayer>(generateUniqueLayerName(*this, "Bitmap layer").c_str());
}


Scene::MaskedBitmapLayer& Scene::newMaskedBitmapLayer(const char* name) {
	return newLayer<MaskedBitmapLayer>(name);
}


Scene::MaskedBitmapLayer& Scene::newMaskedBitmapLayer() {
	return newLayer<MaskedBitmapLayer>(generateUniqueLayerName(*this, "Masked bitmap layer").c_str());
}


Scene::ShapedBitmapLayer& Scene::newShapedBitmapLayer(const char* name) {
	return newLayer<ShapedBitmapLayer>(name);
}


Scene::ShapedBitmapLayer& Scene::newShapedBitmapLayer() {
	return newLayer<ShapedBitmapLayer>(generateUniqueLayerName(*this, "Shaped bitmap layer").c_str());
}


Scene::ShadedBitmapLayer& Scene::newShadedBitmapLayer(const char* name) {
	return newLayer<ShadedBitmapLayer>(name);
}


Scene::ShadedBitmapLayer& Scene::newShadedBitmapLayer() {
	return newLayer<ShadedBitmapLayer>(generateUniqueLayerName(*this, "Shaped bitmap layer").c_str());
}


Scene::SceneLayer& Scene::addScene(const Scene& scene) {
	SceneLayer* l = new SceneLayer(scene);
	l->setName(generateUniqueLayerName(*this, "Scene layer").c_str());
	layers.push_back(l);
	return *l;
}


Scene::Layer* Scene::getLayer(const char* name) const {
	for (auto l : layers)
		if (l->getName() == name)
			return l;
	return nullptr;
}


Scene::Layer& Scene::getLayer(int index) const {
	return *layers[index];
}

Scene::Layer* Scene::getLayer(float x, float y, unsigned int recursionDepth) const {
    for (auto it = layers.crbegin(); it != layers.crend(); it++)
		if (!(*it)->isPhantom()) {
			if ((*it)->getType() == Scene::Layer::Type::SceneLayer) {
				Layer *result = (*it)->getChild(x, y, recursionDepth + 1);
				if (result)
					return result;
			} else {
				if ((*it)->testPoint(x, y))
					return *it;
			}
		}
	return nullptr;
}


int Scene::getLayerIndex(const Layer& layer) const {
	for (size_t i = 0; i < layers.size(); ++i)
		if (layers[i] == &layer)
			return (int)i;
	return -1;
}


int Scene::getLayerCount() const {
	return (int)layers.size();
}


bool Scene::resolveMapping(const Layer& layer, AffineMapping& mapping) const {
	for (auto it = layers.crbegin(); it != layers.crend(); it++) {
		if (&layer == *it) {
			mapping = layer.getMapping();
			return true;
		}

		// scene containers are checked recursively
		if ((*it)->getType() == Scene::Layer::Type::SceneLayer) {
			const SceneLayer& container = (*it)->castTo<SceneLayer>();
			if (container.getScene().resolveMapping(layer, mapping)) {
				mapping = container.getMapping() * mapping;
				return true;
			}
		}
	}
	return false;
}


void Scene::attachLayer(Layer& layer) {
	if (getLayerIndex(layer) >= 0)
		throw SceneIntegrityError(std::string("Layer ") + layer.getName() + " is already in the scene", *this);
	layers.push_back(&layer);
}


Scene::Layer* Scene::detachLayer(int index) {
	Layer* l = layers[index];
	layers.erase(layers.begin() + index);
	return l;
}


Scene::Layer::Layer(Type type) : visible(true), phantom(false), type(type) {}

bool Scene::Layer::testPoint(float x, float y) const {
	return mapping.isPointInside(Point(x, y));
}

Scene::Layer* Scene::Layer::getChild(float, float, unsigned int) const {
	return nullptr;
}


Scene::SceneLayer::SceneLayer(const Scene& scene) : Layer(Type::SceneLayer), scene(scene) {}

bool Scene::SceneLayer::testPoint(float x, float y) const {
	return getChild(x, y) != nullptr;
}

Scene::Layer* Scene::SceneLayer::getChild(float x, float y, unsigned int recursionDepth) const {
	static const unsigned int MAX_RECURSION_DEPTH = 256;
	if (recursionDepth >= MAX_RECURSION_DEPTH)
		return nullptr;
	Point p = mapping.getInverse(x, y);
	return scene.getLayer(p.x, p.y, recursionDepth);
}


Scene::BitmapLayer::BitmapLayer():
	BitmapLayer(Type::BitmapLayer)
{}

Scene::BitmapLayer::BitmapLayer(Type type):
	Layer(type), invAr(0), bitmap(nullptr), bitmapMapping(), modulation(Colors::White)
{}


GL::TextureHandler* Scene::BitmapLayer::resolveContent(RenderingContext& context) {
	if (bitmap)
			context.lockBitmap(bitmap);
	return bitmap;
}

void Scene::BitmapLayer::configure(RenderingContext& context, GL::TextureHandler* content) {
	if (content) {
		invAr = content->getInvAspectRatio();
		context.getProgram().bindSampler(context.getGpu(), *content, "image", TextureParam::INTERP_LINEAR);
	}
	else
		invAr = 0;

	context.getProgram().setVector4("modulationColor", modulation);
}


void Scene::BitmapLayer::render(RenderingContext& context) {
	GL::TextureHandler* content = resolveContent(context);
	if (content) {
		// program selection
		switch (content->getTextureFormat()) {
		case GL::TextureHandler::TextureFormat::OES_Ext:
			context.enableProgram(RenderingPrograms::Program::BLEND_EXT);
			break;
		default:
			context.enableProgram(RenderingPrograms::Program::BLEND);
			break;
		}

		configure(context, content);

		AffineMapping arMapping(context.getMapping() * bitmapMapping);
		arMapping.matrix.scale(1.0f, invAr);
		context.getProgram().setMatrix3(RenderingPrograms::MODELVIEW_MATRIX_ID, arMapping);
		context.blend();
	}
}


bool Scene::BitmapLayer::testPoint(float x, float y) const {
	// no bitmap - no deal (if the source is set to bitmap)
	if (!bitmap)
		return false;
	return (mapping * bitmapMapping).isPointInside(x, y, 1, invAr);
}


Scene::CustomMaskedBitmapLayer::CustomMaskedBitmapLayer(Type type) :
	BitmapLayer(type), maskMapping(), bgColor{ 0, 0, 0, 0 }
{}

void Scene::CustomMaskedBitmapLayer::configure(RenderingContext& context, GL::TextureHandler* content) {
	BitmapLayer::configure(context, content);
	AffineMapping arImgMapping(bitmapMapping), arMaskMapping(maskMapping);
	arImgMapping.matrix.scale(1.0f, invAr);
	arMaskMapping.matrix.scale(1.0f, invAr);
	context.getProgram().setVector4("bgColor", bgColor);
	context.getProgram().setMatrix3(RenderingPrograms::MODELVIEW_MATRIX_ID, context.getMapping());
	context.getProgram().setMatrix3("invImgMapping", arImgMapping.getInverse() * arMaskMapping);
	context.getProgram().setMatrix3("maskMapping", arMaskMapping);
}


Scene::MaskedBitmapLayer::MaskedBitmapLayer() :
	CustomMaskedBitmapLayer(Type::MaskedBitmapLayer), mask(NULL)
{}


void Scene::MaskedBitmapLayer::render(RenderingContext& context) {
	GL::TextureHandler* content = resolveContent(context);
	if (content && mask) {
		const bool mask8bit = mask->getPixelFormat() == PixelFormat::SingleByte;

		// program selection
		switch (content->getTextureFormat()) {
		case GL::TextureHandler::TextureFormat::OES_Ext:
			context.enableProgram(mask8bit ?
				RenderingPrograms::Program::MASKED_8BIT_BLEND_EXT :
				RenderingPrograms::Program::MASKED_BLEND_EXT
			);
			break;
		default:
			context.enableProgram(mask8bit ?
				RenderingPrograms::Program::MASKED_8BIT_BLEND :
				RenderingPrograms::Program::MASKED_BLEND);
			break;
		}

		context.lockBitmap(mask);

		CustomMaskedBitmapLayer::configure(context, content);
		context.bindMask(*mask);
		context.blend();
	}
}


bool Scene::MaskedBitmapLayer::testPoint(float x, float y) const {
	// no bitmap - no deal (if the source is set to bitmap)
	if (!bitmap)
		return false;
	if (mask) {
		RuntimeError::check(mask->isUpToDate(CPU), "Mask is out of date on CPU.");
		const Point p = (mapping * maskMapping).getInverse(x, y);
		int
			w = floorf_fast(mask->getWidth() * p.x),
			h = floorf_fast(mask->getHeight() * p.y);
		if (0 <= w && w < mask->getWidth() && 0 <= h && h < mask->getHeight()) {
			mask->lockPixels(ProcessingTarget::CPU);
			bool result = mask->getPixelInt(w, h) > 0;
			mask->unlockPixels();
			return result;
		}
		return false;
	}
	return BitmapLayer::testPoint(x, y);
}


Scene::ShapedBitmapLayer::ShapedBitmapLayer() :
	CustomMaskedBitmapLayer(Type::ShapedBitmapLayer),
	borderWidth(0), slopeWidth(0), cornerRadius(0), inPixels(true)
{}


void Scene::ShapedBitmapLayer::render(RenderingContext& context) {
	GL::TextureHandler* content = CustomMaskedBitmapLayer::resolveContent(context);

	// program selection
	if (content) {
		switch (content->getTextureFormat()) {
		case GL::TextureHandler::TextureFormat::OES_Ext:
			context.enableProgram(RenderingPrograms::Program::SHAPED_BLEND_EXT);
			break;
		default:
			context.enableProgram(RenderingPrograms::Program::SHAPED_BLEND);
			break;
		}

		CustomMaskedBitmapLayer::configure(context, content);

		// computing border profile in pixels
		AffineMapping arMaskMapping(maskMapping);
		arMaskMapping.matrix.scale(1.0f, invAr);

		Matrix2 mat = context.getMapping().matrix * arMaskMapping.matrix;
		mat.prescale(1.0f, context.getGpu().getOutputResolution().getInvAspectRatio());

		const float scale = inPixels ? context.getOutputWidth() : 1;
		const Point borderProfile(scale * mat.getScalingX(), scale * mat.getScalingY());
		context.getProgram().setVector2("borderProfile", borderProfile.x, borderProfile.y);
		context.getProgram().setFloat("cornerRadius", cornerRadius + borderWidth);

		context.blend();
	}
}


bool Scene::ShapedBitmapLayer::testPoint(float x, float y) const {
	// no bitmap - no deal (if the source is set to bitmap)
	if (!bitmap)
		return false;
	return (mapping * maskMapping).isPointInside(x, y, 1, invAr) && BitmapLayer::testPoint(x, y);
}


Scene::ShadedBitmapLayer::ShadedBitmapLayer() :
	BitmapLayer(Type::ShadedBitmapLayer),
	shader(nullptr)
{}


void Scene::ShadedBitmapLayer::render(RenderingContext& context) {
	if (!shader)
		return;
	GL::TextureHandler* content = BitmapLayer::resolveContent(context);

	if (content)
		invAr = content->getInvAspectRatio();
	else
		invAr = 0;

	AffineMapping arMapping(context.getMapping());
	if (content)
		arMapping.matrix.scale(1.0f, invAr);

	shader->prepare(context.getGpu(), content, nullptr, arMapping);
	context.blend();
}

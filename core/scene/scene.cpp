#include "../basic_types.h"
#include "../scene/scene.h"
#include "../exception.h"
#include <map>
#include <vector>
#include <cmath>

#include "../debug.h"

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
	while (scene.getLayer(candidate = prefix + (" #" + to_string(n))))
		n++;
	return candidate;
}


Scene::Scene() {}


Scene::~Scene() {
	for (auto it : layers)
		delete it;
}


Scene::BitmapLayer& Scene::newBitmapLayer(const string& name) {
	return newLayer<BitmapLayer>(name);
}


Scene::BitmapLayer& Scene::newBitmapLayer() {
	return newLayer<BitmapLayer>(generateUniqueLayerName(*this, "Bitmap layer"));
}


Scene::MaskedBitmapLayer& Scene::newMaskedBitmapLayer(const string& name) {
	return newLayer<MaskedBitmapLayer>(name);
}


Scene::MaskedBitmapLayer& Scene::newMaskedBitmapLayer() {
	return newLayer<MaskedBitmapLayer>(generateUniqueLayerName(*this, "Masked bitmap layer"));
}


Scene::ShapedBitmapLayer& Scene::newShapedBitmapLayer(const string& name) {
	return newLayer<ShapedBitmapLayer>(name);
}


Scene::ShapedBitmapLayer& Scene::newShapedBitmapLayer() {
	return newLayer<ShapedBitmapLayer>(generateUniqueLayerName(*this, "Shaped bitmap layer"));
}


Scene::ShadedBitmapLayer& Scene::newShadedBitmapLayer(const string& name) {
	return newLayer<ShadedBitmapLayer>(name);
}


Scene::ShadedBitmapLayer& Scene::newShadedBitmapLayer() {
	return newLayer<ShadedBitmapLayer>(generateUniqueLayerName(*this, "Shaped bitmap layer"));
}


Scene::SceneLayer& Scene::addScene(const Scene& scene) {
	SceneLayer* l = new SceneLayer(scene);
	l->setName(generateUniqueLayerName(*this, "Scene layer"));
	layers.push_back(l);
	return *l;
}


Scene::Layer* Scene::getLayer(std::string name) const {
	for (auto l : layers)
		if (l->getName() == name)
			return l;
	return nullptr;
}


Scene::Layer& Scene::getLayer(int index) const {
	return *layers[index];
}

Scene::Layer* Scene::getLayer(float x, float y, unsigned int recursionDepth) const {
	for (int i = layers.size() - 1; i >= 0; --i)
		if (!layers[i]->isPhantom())
			if (layers[i]->getType() == Scene::Layer::Type::SceneLayer) {
				Layer* result = layers[i]->getChild(x, y, recursionDepth + 1);
				if (result)
					return result;
			}
			else
				if (layers[i]->testPoint(x, y))
					return layers[i];
	return nullptr;
}


int Scene::getLayerIndex(const Layer& layer) const {
	for (size_t i = 0; i < layers.size(); ++i)
		if (layers[i] == &layer)
			return i;
	return -1;
}


int Scene::getLayerCount() const {
	return (int)layers.size();
}


bool Scene::resolveMapping(const Layer& layer, AffineMapping& mapping) const {
	for (size_t i = layers.size() - 1; i >= 0; i--) {
		if (&layer == layers[i]) {
			mapping = layer.getMapping();
			return true;
		}

		// scene containers are checked recursively
		if (layers[i]->getType() == Scene::Layer::Type::SceneLayer) {
			const SceneLayer& container = layers[i]->castTo<SceneLayer>();
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
		throw SceneIntegrityError("Layer " + layer.getName() + " is already in the scene", *this);
	layers.push_back(&layer);
}


Scene::Layer* Scene::detachLayer(int index) {
	Layer* l = layers[index];
	layers.erase(layers.begin() + index);
	return l;
}


Scene::Layer::Layer(Type type) : type(type), visible(true), phantom(false) {}

bool Scene::Layer::testPoint(float x, float y) const {
	return mapping.isPointInside(Point(x, y));
}

Scene::Layer* Scene::Layer::getChild(float, float, unsigned int) const {
	return NULL;
}


Scene::SceneLayer::SceneLayer(const Scene& scene) : Layer(Type::SceneLayer), scene(scene) {}

bool Scene::SceneLayer::testPoint(float x, float y) const {
	return getChild(x, y) != NULL;
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
	Layer(type), source(ImageSource::BITMAP), invAr(0), bitmap(NULL), bitmapMapping(), modulation(1.0f, 1.0f, 1.0f, 1.0f)
{}


bool Scene::BitmapLayer::testPoint(float x, float y) const {
	// no bitmap - no deal (if the source is set to bitmap)
	if (!bitmap && source == ImageSource::BITMAP)
		return false;
	return (mapping * bitmapMapping).isPointInside(x, y, 1, invAr);
}


Scene::CustomMaskedBitmapLayer::CustomMaskedBitmapLayer(Type type) :
	BitmapLayer(type), maskMapping(), bgColor(0,0,0,0)
{}


Scene::MaskedBitmapLayer::MaskedBitmapLayer() :
	CustomMaskedBitmapLayer(Type::MaskedBitmapLayer), mask(NULL)
{}


bool Scene::MaskedBitmapLayer::testPoint(float x, float y) const {
	// no bitmap - no deal (if the source is set to bitmap)
	if (!bitmap && source == ImageSource::BITMAP)
		return false;
	if (mask) {
		if (!mask->isUpToDate(CPU))
			BEATMUP_ERROR("CPU version of the mask is out of date.");
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
	slopeWidth(0), borderWidth(0), cornerRadius(0), inPixels(true)
{}


bool Scene::ShapedBitmapLayer::testPoint(float x, float y) const {
	// no bitmap - no deal (if the source is set to bitmap)
	if (!bitmap && source == ImageSource::BITMAP)
		return false;
	return (mapping * maskMapping).isPointInside(x, y, 1, invAr) && BitmapLayer::testPoint(x, y);
}


Scene::ShadedBitmapLayer::ShadedBitmapLayer() :
	BitmapLayer(Type::ShadedBitmapLayer),
	layerShader(nullptr)
{}
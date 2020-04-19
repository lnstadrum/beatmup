#include "java_factory.h"
#include <core/geometry.h>

JavaFactory::JavaFactory() : usedEnv(NULL) {}

void JavaFactory::initialize(JNIEnv* jenv) {
    if (usedEnv == jenv)
        return;
    usedEnv = jenv;
    // AffineMapping
    jclass cls = jenv->FindClass("Beatmup/Geometry/AffineMapping");
    affineMapping.constructor = jenv->GetMethodID(cls, "<init>", "()V");
    affineMapping.ida11 = jenv->GetFieldID(cls, "a11", "F");
    affineMapping.ida12 = jenv->GetFieldID(cls, "a12", "F");
    affineMapping.ida21 = jenv->GetFieldID(cls, "a21", "F");
    affineMapping.ida22 = jenv->GetFieldID(cls, "a22", "F");
    affineMapping.idx = jenv->GetFieldID(cls, "x", "F");
    affineMapping.idy = jenv->GetFieldID(cls, "y", "F");
    jenv->DeleteLocalRef(cls);
    // IntRectangle
    cls = jenv->FindClass("Beatmup/Geometry/IntRectangle");
    intRectangle.constructor = jenv->GetMethodID(cls, "<init>", "(IIII)V");
    intRectangle.ix1 = jenv->GetFieldID(cls, "x1", "I");
    intRectangle.iy1 = jenv->GetFieldID(cls, "y1", "I");
    intRectangle.ix2 = jenv->GetFieldID(cls, "x2", "I");
    intRectangle.iy2 = jenv->GetFieldID(cls, "y2", "I");
    jenv->DeleteLocalRef(cls);
    // IntPoint
    cls = jenv->FindClass("Beatmup/Geometry/IntPoint");
    intPoint.constructor = jenv->GetMethodID(cls, "<init>", "(II)V");
    intPoint.ix = jenv->GetFieldID(cls, "x", "I");
    intPoint.iy = jenv->GetFieldID(cls, "y", "I");
    jenv->DeleteLocalRef(cls);
    // Color
    cls = jenv->FindClass("Beatmup/Imaging/Color");
    color.constructor = jenv->GetMethodID(cls, "<init>", "(IIII)V");
    color.idr = jenv->GetFieldID(cls, "r", "I");
    color.idg = jenv->GetFieldID(cls, "g", "I");
    color.idb = jenv->GetFieldID(cls, "b", "I");
    color.ida = jenv->GetFieldID(cls, "a", "I");
    jenv->DeleteLocalRef(cls);
}


jobject JavaFactory::makeAffineMapping(JNIEnv *jenv, const Beatmup::AffineMapping &mapping) {
    initialize(jenv);
    jclass cls = jenv->FindClass("Beatmup/Geometry/AffineMapping");
    jobject result = jenv->NewObject(cls, affineMapping.constructor);
    jenv->DeleteLocalRef(cls);
    setAffineMapping(jenv, mapping, result);
    return result;
}


void JavaFactory::setAffineMapping(JNIEnv* jenv, const Beatmup::AffineMapping& mapping, jobject jMapping) {
    initialize(jenv);
    float a11, a12, a21, a22;
    mapping.matrix.getElements(a11,a12,a21,a22);
    jenv->SetFloatField(jMapping, affineMapping.ida11, a11);
    jenv->SetFloatField(jMapping, affineMapping.ida12, a12);
    jenv->SetFloatField(jMapping, affineMapping.ida21, a21);
    jenv->SetFloatField(jMapping, affineMapping.ida22, a22);
    jenv->SetFloatField(jMapping, affineMapping.idx, mapping.position.x);
    jenv->SetFloatField(jMapping, affineMapping.idy, mapping.position.y);
}

jobject JavaFactory::makeIntRectangle(JNIEnv *jenv, const Beatmup::IntRectangle r) {
    initialize(jenv);
    jclass cls = jenv->FindClass("Beatmup/Geometry/IntRectangle");
    jobject result = jenv->NewObject(cls, intRectangle.constructor, r.a.x, r.a.y, r.b.x, r.b.y);
    jenv->DeleteLocalRef(cls);
    return result;
}


jobject JavaFactory::makeIntPoint(JNIEnv *jenv, const Beatmup::IntPoint p) {
    initialize(jenv);
    jclass cls = jenv->FindClass("Beatmup/Geometry/IntPoint");
    jobject result = jenv->NewObject(cls, intPoint.constructor, p.x, p.y);
    jenv->DeleteLocalRef(cls);
    return result;
}


void JavaFactory::setIntPoint(JNIEnv *jenv, const Beatmup::IntPoint &point, jobject jPoint) {
    initialize(jenv);
    jenv->SetIntField(jPoint, intPoint.ix, point.x);
    jenv->SetIntField(jPoint, intPoint.iy, point.y);
}


void JavaFactory::setIntPoint(JNIEnv *jenv, int x, int y, jobject jPoint) {
    initialize(jenv);
    jenv->SetIntField(jPoint, intPoint.ix, x);
    jenv->SetIntField(jPoint, intPoint.iy, y);
}


jobject JavaFactory::makeColor(JNIEnv *jenv, const Beatmup::color4i& c) {
    initialize(jenv);
    jclass cls = jenv->FindClass("Beatmup/Imaging/Color");
    jobject result = jenv->NewObject(cls, color.constructor, c.r, c.g, c.b, c.a);
    jenv->DeleteLocalRef(cls);
    return result;
}


void JavaFactory::setColor(JNIEnv* jenv, const Beatmup::color4i& c, jobject jColor) {
    initialize(jenv);
    jenv->SetIntField(jColor, color.idr, c.r);
    jenv->SetIntField(jColor, color.idg, c.g);
    jenv->SetIntField(jColor, color.idb, c.b);
    jenv->SetIntField(jColor, color.ida, c.a);
}
#pragma once
#include <jni.h>
#include <core/geometry.h>


class JavaFactory {
private:
    JNIEnv* usedEnv;        //!< last used JNIEnv

    struct {
        jmethodID constructor;
        jfieldID ida11,ida12,ida21,ida22, idx,idy;
    } affineMapping;

    struct {
        jmethodID constructor;
        jfieldID ix1, iy1, ix2, iy2;
    } intRectangle;

    struct {
        jmethodID constructor;
        jfieldID ix, iy;
    } intPoint;

    struct {
        jmethodID constructor;
        jfieldID idr, idg, idb, ida;
    } color;

    void initialize(JNIEnv* jenv);
public:
    JavaFactory();

    /**
     * \brief Creates Java's AffineMapping object from Beatmup AffineMapping
     */
    jobject makeAffineMapping(
            JNIEnv *jenv,
            const Beatmup::AffineMapping &mapping
    );

    void setAffineMapping(
            JNIEnv* jenv,
            const Beatmup::AffineMapping& mapping,
            jobject jMapping
    );

    /**
     * \brief Creates Java's IntRectangle object from Beatmup IntRectangle
     */
    jobject makeIntRectangle(JNIEnv *jenv, const Beatmup::IntRectangle);

    jobject makeIntPoint(JNIEnv *jenv, const Beatmup::IntPoint);

    void setIntPoint(
            JNIEnv* jenv,
            const Beatmup::IntPoint& point,
            jobject jPoint
    );

    void setIntPoint(
            JNIEnv* jenv,
            int x, int y,
            jobject jPoint
    );

    /**
     * \brief Creates Java's Color object from Beatmup's color
     */
    jobject makeColor(JNIEnv *jenv, const Beatmup::color4i& color);

    void setColor(JNIEnv* jenv, const Beatmup::color4i& c, jobject jColor);

};
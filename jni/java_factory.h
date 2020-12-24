/*
    Beatmup image and signal processing library
    Copyright (C) 2019, lnstadrum

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <jni.h>
#include <core/geometry.h>


/**
    Set of utility functions for Java bindings.
*/
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
#include "gl_utils.h"
#include <android/native_window_jni.h>
#include <core/gpu/pipeline.h>
#include <core/gpu/display_switch.h>


bool Beatmup::Android::switchDisplay(Beatmup::Environment& env, JNIEnv* jenv, jobject surface) {
    bool result = false;
    if (surface) {
        ANativeWindow *wnd = ANativeWindow_fromSurface(jenv, surface);
        if (!wnd)
            throw RuntimeError("Empty surface window got when switching GL display.");
        result = Beatmup::DisplaySwitch::run(env, wnd);
        ANativeWindow_release(wnd);
    } else
        result = Beatmup::DisplaySwitch::run(env, NULL);
    return result;
}
# Add project specific ProGuard rules here.
# You can control the set of applied configuration files using the
# proguardFiles setting in build.gradle.
#
# For more details, see
#   http://developer.android.com/guide/developing/tools/proguard.html

# If your project uses WebView with JS, uncomment the following
# and specify the fully qualified class name to the JavaScript interface
# class:
#-keepclassmembers class fqcn.of.javascript.interface.for.webview {
#   public *;
#}

# Uncomment this to preserve the line number information for
# debugging stack traces.
#-keepattributes SourceFile,LineNumberTable

# If you keep the line number information, uncomment this to
# hide the original source file name.
#-renamesourcefileattribute SourceFile

-keepattributes InnerClasses

-keep,includedescriptorclasses class Beatmup.* {
    public *;
}

-keep class Beatmup.Scene$* {
    public *;
}

-keep,includedescriptorclasses class Beatmup.Filters.** {
    public *;
}

-keep,includedescriptorclasses class Beatmup.Geometry.* {
    public *;
}

-keep,includedescriptorclasses class Beatmup.Visual.* {
    public *;
}

-keepclassmembers class Beatmup.AndroidUITask {
    protected boolean iterationCancelled();
}

#
#   Following methods/fields are kept as they are addressed from JNI layer
#

-keepclassmembers class Beatmup.Object {
    protected int id;
}

-keepclassmembers class Beatmup.Context {
    private int envId;
    private java.lang.Object glSurface;
}

-keepclassmembers class Beatmup.Task {
    protected boolean taskDone(boolean);
}

-keepclassmembers class Beatmup.AndroidContext {
    private void updateCamTextureCallback();
    private void initCamTextureCallback(int);
}
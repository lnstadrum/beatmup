javah -d ../../../../../jni/jniheaders -classpath ~/Android/Sdk/platforms/android-26/android.jar:../../../build/intermediates/classes/full/debug\
  Beatmup.Object\
  Beatmup.Context\
  Beatmup.Android.Context\
  Beatmup.Android.ExternalBitmap\
  Beatmup.Sequence\
  Beatmup.Visual.Android.BasicDisplay\
  Beatmup.Rendering.Scene\
  Beatmup.Rendering.SceneRenderer\
  Beatmup.Shading.Shader\
  Beatmup.Shading.ShaderApplicator\
  Beatmup.Bitmap\
  Beatmup.Imaging.BinaryOperation\
  Beatmup.Imaging.FloodFill\
  Beatmup.Imaging.Filters.Local.PixelwiseFilter\
  Beatmup.Imaging.Filters.Local.ColorMatrixTransform\
  Beatmup.Imaging.Filters.ImageTuning\
  Beatmup.Imaging.Filters.Local.Sepia\
  Beatmup.Imaging.Filters.Resampler\
  Beatmup.Imaging.ColorMatrix\
  Beatmup.Audio.Signal\
  Beatmup.Audio.Playback\
  Beatmup.Audio.Source.Harmonic\
  Beatmup.Utils.VariablesBundle\
  Beatmup.Pipelining.CustomPipeline\
  Beatmup.Pipelining.Multitask\
  Beatmup.NNets.GPUBenchmark


# Regexp:
#   JNIEXPORT ([^\s]+) JNICALL ([^\s]+)_([^\s]+)\n\s+\(JNIEnv \*,([^\)]+)\);
#   JNIMETHOD($1, $3, $2, $3)\n    (JNIEnv * jenv,$4)\n{\n    BEATMUP_ENTER;\n}\n\n

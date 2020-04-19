javah -d ../../../../../jni/jniheaders -classpath ~/Android/Sdk/platforms/android-26/android.jar:../../../build/intermediates/classes/full/release\
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
  Beatmup.Audio.SignalPlot\
  Beatmup.Audio.Playback\
  Beatmup.Audio.Source.Harmonic\
  Beatmup.Utils.VariablesBundle\
  Beatmup.Pipelining.CustomPipeline\
  Beatmup.Pipelining.Multitask\
  Beatmup.Pipelining.TaskHolder\
  Beatmup.NNets.GPUBenchmark


# Regexp:
#   (/\*\n(\s\*.*\n)+\s\*/\n)?JNIEXPORT ([^\s]+) JNICALL ([^\s]+)_([^\s]+)\n\s+\(JNIEnv \*,([^\)]+)\);
#   JNIMETHOD($3, $5, $4, $5)\n    (JNIEnv * jenv,$6)\n{\n    BEATMUP_ENTER;\n}\n

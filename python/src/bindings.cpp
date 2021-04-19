/*
    Beatmup image and signal processing library
    Copyright (C) 2020, lnstadrum

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

#include <stdexcept>
#include <vector>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/eval.h>
#include <pybind11/stl.h>

#include "context.h"
#include "bitmap/metric.h"
#include "bitmap/resampler.h"
#include "bitmap/tools.h"
#include "contours/contours.h"
#include "filters/color_matrix.h"
#include "filters/pixelwise_filter.h"
#include "filters/sepia.h"
#include "gpu/swapper.h"
#include "gpu/variables_bundle.h"
#include "masking/flood_fill.h"
#include "nnets/conv2d.h"
#include "nnets/classifier.h"
#include "nnets/deserialized_model.h"
#include "nnets/dense.h"
#include "nnets/image_sampler.h"
#include "nnets/pooling2d.h"
#include "nnets/softmax.h"
#include "pipelining/custom_pipeline.h"
#include "pipelining/multitask.h"
#include "scene/renderer.h"
#include "scene/scene.h"
#include "shading/image_shader.h"
#include "shading/shader_applicator.h"

#include "binding_tools.hpp"
#include "bitmap.h"
#include "chunk_collection.h"


namespace py = pybind11;
using namespace Beatmup;


PYBIND11_MODULE(beatmup, module) {
    module.doc() = R"doc(
        beatmup module
        --------------

        .. autosummary::
            :toctree: python/_generate

            AbstractBitmap
            AbstractTask
            AffineMapping
            Bitmap
            BitmapResampler
            ChunkCollection
            ChunkFile
            Context
            CustomPipeline
            FloodFill
            ImageShader
            IntegerContour2D
            InternalBitmap
            Metric
            Multitask
            PixelFormat
            Scene
            SceneRenderer
            ShaderApplicator
            WritableChunkCollection
    )doc";

    auto gl = module.def_submodule("gl", R"doc(
        beatmup.gl module
        -----------------

        .. autosummary::
            :toctree: python/_generate

            TextureHandler
            VariablesBundle
    )doc");

    auto filters = module.def_submodule("filters", R"doc(
        beatmup.filters module
        ----------------------

        .. autosummary::
            :toctree: python/_generate

            ColorMatrix
            PixelwiseFilter
            Sepia
    )doc");

    auto nnets = module.def_submodule("nnets", R"doc(
        beatmup.nnets module
        --------------------

        .. autosummary::
            :toctree: python/_generate

            ActivationFunction
            AbstractOperation
            Classifier
            Conv2D
            Dense
            DeserializedModel
            ImageSampler
            InferenceTask
            Model
            Padding
            Pooling2D
            Softmax
    )doc");

    module.def("say_hi", []() {
            Context ctx;
            py::print("Beatmup is up and running, yay!");
            py::exec("import platform; print('Python version:', platform.python_version())");
        },
        "Prints some greetings");

    /**
     * PixelFormat
     */
    py::enum_<PixelFormat>(module, "PixelFormat", "Specifies bitmap pixel format")
        .value("SINGLE_BYTE",     PixelFormat::SingleByte,     "single channel of 8 bits per pixel (like grayscale), unsigned integer values")
        .value("TRIPLE_BYTE",     PixelFormat::TripleByte,     "3 channels of 8 bits per pixel (like RGB), unsigned integer values")
        .value("QUAD_BYTE",       PixelFormat::QuadByte,       "4 channels of 8 bits per pixel (like RGBA), unsigned integer values")
        .value("SINGLE_FLOAT",    PixelFormat::SingleFloat,    "single channel of 32 bits per pixel (like grayscale), single precision floating point values")
        .value("TRIPLE_FLOAT",    PixelFormat::TripleFloat,    "3 channels of 32 bits per pixel, single precision floating point values")
        .value("QUAD_FLOAT",      PixelFormat::QuadFloat,      "4 channels of 32 bits per pixel, single precision floating point values")
        .value("BINARY_MASK",     PixelFormat::BinaryMask,     "1 bit per pixel")
        .value("QUATERNARY_MASK", PixelFormat::QuaternaryMask, "2 bits per pixel")
        .value("HEX_MASK",        PixelFormat::HexMask,        "4 bits per pixel")
        .export_values();

    /**
     * GL::TextureHandler
     */
    py::class_<GL::TextureHandler>(gl, "TextureHandler",
            "A texture stored in GPU memory")

        .def("get_width", &GL::TextureHandler::getWidth,
            "Returns width of the texture in pixels")

        .def("get_height", &GL::TextureHandler::getHeight,
            "Returns height of the texture in pixels")

        .def("get_depth", &GL::TextureHandler::getDepth,
            "Returns depth of the texture in pixels")

        .def("get_number_of_channels", &GL::TextureHandler::getNumberOfChannels,
            "Returns number of channels containing in the texture");

    /**
     * AbstractTask
     */
    py::class_<AbstractTask>(module, "AbstractTask", "Abstract task executable in a thread pool of a Context");

    /**
     * Context
     */
    py::class_<Context>(module, "Context", "Beatmup engine context")

        .def(py::init<>())

        .def(py::init<const PoolIndex>())

        .def("perform_task", &Context::performTask,
            py::arg("task"), py::arg("pool") = 0,
            "Performs a given task. Returns its execution time in milliseconds")

        .def("repeat_task", &Context::repeatTask,
            py::arg("task"), py::arg("abort_current"), py::arg("pool") = 0,
            py::keep_alive<1, 2>(),     // context alive => task alive
            R"doc(
                Ensures a given task executed at least once

                :param task:            The task
                :param abort_current:   If True and the same task is currently running, the abort signal is sent.
                :param pool:            A thread pool to run the task in
            )doc")

        .def("submit_task", &Context::submitTask,
            py::arg("task"), py::arg("pool") = 0,
            py::keep_alive<1, 2>(),     // context alive => task alive
            "Adds a new task to the jobs queue")

        .def("submit_persistent_task", &Context::submitPersistentTask,
            py::arg("task"), py::arg("pool") = 0,
            py::keep_alive<1, 2>(),     // context alive => task alive
            "Adds a new persistent task to the jobs queue")

        .def("wait_for_job", &Context::waitForJob,
            py::arg("job"), py::arg("pool") = 0,
            "Blocks until a given job finishes")

        .def("abort_job", &Context::abortJob,
            py::arg("job"), py::arg("pool") = 0,
            "Aborts a given submitted job.")

        .def("wait", &Context::wait,
            "Blocks until all the submitted jobs are executed",
            py::arg("pool") = 0)

        .def("busy", &Context::busy,
            "Returns `True` if a specific thread pool in the context is executing a Task",
            py::arg("pool") = 0)

        .def("check", &Context::check,
            "Checks if a specific thread pool is doing great: rethrows exceptions occurred during tasks execution, if any.",
            py::arg("pool") = 0)

        .def("max_allowed_worker_count", &Context::maxAllowedWorkerCount,
            "Returns maximum number of working threads per task in a given thread pool",
            py::arg("pool") = 0)

        .def("limit_worker_count", &Context::limitWorkerCount,
            "Limits maximum number of threads (workers) when performing tasks in a given pool",
            py::arg("max_value"), py::arg("pool") = 0)

        .def("is_gpu_queried", &Context::isGpuQueried,
            "Returns `True` if GPU was queried and ready to use")

        .def("is_gpu_ready", &Context::isGpuReady,
            "Returns `True` if GPU was queried and ready to use")

        .def("warm_up_gpu", &Context::warmUpGpu, R"doc(
            Initializes GPU within a given Context if not yet (takes no effect if it already is).
            GPU initialization may take some time and is done when a first task using GPU is being run. Warming up
            the GPU is useful to avoid the app get stuck for some time when it launches its first task on GPU.
        )doc")

        .def("query_gpu_info", [](Context &ctx) -> py::object {
            std::string vendor, renderer;
            if (ctx.queryGpuInfo(vendor, renderer))
                return py::make_tuple<>(vendor, renderer);
            return py::none();
        },
            "Queries information about GPU and returns a tuple of vendor and renderer strings, or None if no GPU available.")

        .def("empty_gpu_recycle_bin", [](Context& ctx) {
                auto* bin = ctx.getGpuRecycleBin();
                if (bin)
                    bin->emptyBin();
            }, R"doc(
            Empties GPU recycle bin.
            When a bitmap is destroyed in the application code, its GPU storage is not destroyed immediately. This is due to the fact that destroying a
            texture representing the bitmap content in the GPU memory needs to be done in a thread that has access to the GPU, which is one of the
            threads in the thread pool. The textures of destroyed bitmaps are marked as unused anymore and put into a "GPU trash bin". The latter is
            emptied by calling this function.
            In applications doing repeated allocations and deallocations of images (e.g., processing video frames in a loop), it is recommended to empty
            the GPU recycle bin periodically in the described way in order to prevent running out of memory.
        )doc");

    /**
     * AbstractBitmap
     */
    py::class_<AbstractBitmap, GL::TextureHandler>(module, "AbstractBitmap",
            "Abstract bitmap class")

        .def("get_pixel_format", &AbstractBitmap::getPixelFormat,
            "Returns pixel format of the bitmap")

        .def("get_memory_size", &AbstractBitmap::getMemorySize,
            "Returns bitmap size in bytes")

        .def("get_context", &AbstractBitmap::getContext,
            "Returns Context the current bitmap is attached to")

        .def("zero", &AbstractBitmap::zero,
            "Sets all the pixels to zero")

        .def("__str__", &AbstractBitmap::toString,
            "Returns a string describing the bitmap")

        .def("save_bmp", &AbstractBitmap::saveBmp, py::arg("filename"),
            "Saves a bitmap to a BMP file");

    /**
     * InternalBitmap
     */
    py::class_<InternalBitmap, AbstractBitmap>(module, "InternalBitmap", R"doc(
            Bitmap whose memory is managed by the Beatmup engine.
            Main pixel data container used internally by Beatmup. Applications would typically use a different incarnation
            of AbstractBitmap implementing I/O operations, and InternalBitmap instances are used to exchange data between
            different processing entities (AbstractTask instances) within the application.
        )doc")

        .def(py::init<Context&, PixelFormat, int, int, bool>(),
            py::arg("context"), py::arg("pixel_format"), py::arg("width"), py::arg("height"), py::arg("allocate") = true,
            py::keep_alive<1, 2>())      // bitmap alive => context alive

        .def(py::init<Context&, const char*>(),
            py::keep_alive<1, 2>());     // bitmap alive => context alive

    /**
     * Python::Bitmap
     */
    py::class_<Python::Bitmap, AbstractBitmap>(module, "Bitmap", py::buffer_protocol(),
            "A bitmap wrapping a numpy container without copying")

        .def(py::init<Beatmup::Context&, py::buffer&>(),
            py::keep_alive<1, 2>())      // bitmap alive => context alive

        .def_buffer([](Python::Bitmap& bitmap) {
            Swapper::pullPixels(bitmap);
            return bitmap.getPythonBuffer();
        });

    /**
     * Tools
     */
    module.def_submodule("bitmaptools")
        .def("make_copy", (InternalBitmap* (*)(AbstractBitmap&, Context&, PixelFormat))&BitmapTools::makeCopy,
            py::arg("bitmap"), py::arg("context"),  py::arg("format"),
            py::return_value_policy::take_ownership,
            py::keep_alive<0, 1>(),     // bitmap alive => context alive
            R"doc(
                Makes a copy of a bitmap for a given Context converting the data to a given pixel format.
                Can be used to exchange image content between different instances of Context.
                The copy is done in an AbstractTask run in the default thread pool of the source bitmap context.

                :param bitmap:       the bitmap to copy
                :param context:      the Context instance the copy is associated with
                :param format:       pixel format of the copy
            )doc")

        .def("chessboard", &BitmapTools::chessboard,
            py::arg("context"), py::arg("width"), py::arg("height"), py::arg("cell_size"), py::arg("format") = PixelFormat::BinaryMask,
            py::return_value_policy::take_ownership,
            py::keep_alive<0, 1>(),     // bitmap alive => context alive
            R"doc(
                Renders a chessboard image.

                :param context:      a Context instance
                :param width:        width in pixels of the resulting bitmap
                :param height:       height in pixels of the resulting bitmap
                :param cell_size:    size of a single chessboard cell in pixels
                :param pixel_format: pixel format of the resulting bitmap
            )doc")

        .def("noise", [](AbstractBitmap& bitmap) { BitmapTools::noise(bitmap); },
            py::arg("bitmap"),
            "Fills a given bitmap with random noise.")

        .def("noise", [](AbstractBitmap& bitmap, const py::tuple& area) { BitmapTools::noise(bitmap, Python::toRectangle<int>(area)); },
            py::arg("bitmap"), py::arg("area"),
            "Replaces a rectangular area in a bitmap by random noise.")

        .def("make_opaque", [](AbstractBitmap& bitmap, const py::tuple& area) {
                BitmapTools::makeOpaque(bitmap, Python::toRectangle<int>(area));
            },
            py::arg("bitmap"), py::arg("area"),
            "Makes a bitmap area opaque")

        .def("invert", &BitmapTools::invert,
            py::arg("input"), py::arg("output"),
            "Inverses colors of an image in a pixelwise fashion")

        .def("scanline_search", [](AbstractBitmap& bitmap, const py::tuple& value, const py::tuple& startFrom) -> py::object {
                auto pt = BitmapTools::scanlineSearch(bitmap, Python::toPixfloat4(value), Python::toPoint<int>(startFrom));
                if (pt.x == -1 && pt.y == -1)
                    return py::none();
                return Python::toTuple(pt);
            },
            py::arg("bitmap"), py::arg("value"),  py::arg("start_from") = Python::toTuple(IntPoint::ZERO),
            R"doc(
                Goes through a bitmap in scanline order (left to right, top to bottom) until a pixel of a given color is met.

                :param source:        the bitmap to scan
                :param value:         the color value to look for
                :param start_from:    starting pixel position

                Returns the next closest position of the searched value (in scanline order) or None if not found.
            )doc");

    /**
     * BitmapResampler
     */
    py::class_<BitmapResampler, AbstractTask> bitmapResampler(module, "BitmapResampler", R"doc(
            Resamples an image to a given resolution.
            Implements different resampling approaches, including standard ones (bilinear, bicubic, etc.) and a neural network-based 2x upsampling approach dubbed as "x2".
        )doc");

        bitmapResampler.def(py::init<Context&>(), py::arg("context"),
            py::keep_alive<1, 2>())     // resampler alive => context alive

        .def_property("input", &BitmapResampler::getInput,
            py::cpp_function(&BitmapResampler::setInput, py::keep_alive<1, 2, 1>()),     // instance alive => bitmap alive
            "Input bitmap")

        .def_property("output", &BitmapResampler::getOutput,
            py::cpp_function(&BitmapResampler::setOutput, py::keep_alive<1, 2, 2>()),     // instance alive => bitmap alive
            "Output bitmap")

        .def_property("mode", &BitmapResampler::getMode, &BitmapResampler::setMode, "Resmpling algorithm (mode)")

        .def_property("cubic_parameter", &BitmapResampler::setCubicParameter, &BitmapResampler::getCubicParameter,
            "Cubic resampling parameter (`alpha`)")

        .def_property("input_rectangle", [](BitmapResampler& resampler, const py::tuple& area) {
                resampler.setInputRect(Python::toRectangle<int>(area));
            },
            [](BitmapResampler& resampler) {
                return Python::toTuple(resampler.getInputRect());
            },
            "Specifies a rectangular working area in the input bitmap. Pixels outside of this area are not used.")

        .def_property("output_rectangle", [](BitmapResampler& resampler, const py::tuple& area) {
                resampler.setOutputRect(Python::toRectangle<int>(area));
            },
            [](BitmapResampler& resampler) {
                return Python::toTuple(resampler.getOutputRect());
            },
            "Specifies a rectangular working area in the output bitmap. Pixels outside of this area are not affected.");

    py::enum_<BitmapResampler::Mode>(bitmapResampler, "Mode", "Resampling mode (algorithm) specification")
        .value("NEAREST_NEIGHBOR", BitmapResampler::Mode::NEAREST_NEIGHBOR, "zero-order: usual nearest neighbor")
        .value("BOX",              BitmapResampler::Mode::BOX,              "'0.5-order': anti-aliasing box filter; identical to nearest neighbor when upsampling")
        .value("LINEAR",           BitmapResampler::Mode::LINEAR,           "first order: bilinear interpolation")
        .value("CUBIC",            BitmapResampler::Mode::CUBIC,            "third order: bicubic interpolation")
        .value("CONVNET",          BitmapResampler::Mode::CONVNET,          "upsampling x2 using a convolutional neural network")
        .export_values();

    /**
     * Filters::PixelwiseFilter
     */
    py::class_<Filters::PixelwiseFilter, AbstractTask>(filters, "PixelwiseFilter",
        "Base class for image filters processing a given bitmap in a pixelwise fashion.")

        .def_property("input",
            &Filters::PixelwiseFilter::getInput,
            py::cpp_function(&Filters::PixelwiseFilter::setInput, py::keep_alive<1, 2, 1>()),     // instance alive => bitmap alive
            "Input bitmap")

        .def_property("output",
            &Filters::PixelwiseFilter::getOutput,
            py::cpp_function(&Filters::PixelwiseFilter::setOutput, py::keep_alive<1, 2, 2>()),     // instance alive => bitmap alive
            "Output bitmap");

    /**
     * Filters::ColorMatrix
     */
    py::class_<Filters::ColorMatrix, Filters::PixelwiseFilter>(filters, "ColorMatrix",
            "Color matrix filter: applies an affine mapping Ax + B at each pixel of a given image in RGBA space")

        .def(py::init<>())

        .def("set_coefficients", [](Filters::ColorMatrix& colorMatrix, int ouch, float bias, const py::tuple& rgba) {
                const color4f c = Python::toColor4f(rgba);
                colorMatrix.setCoefficients(ouch, bias, c.r, c.g, c.b, c.a);
            },
            "Sets color matrix coefficients for a specific output color channel",
            py::arg("out_channel"), py::arg("add"), py::arg("rgba"))

        .def("set_hsv_correction", &Filters::ColorMatrix::setHSVCorrection,
            "Resets the current transformation to a matrix performing standard HSV correction",
            py::arg("hue_shift_degrees"), py::arg("saturation_factor"), py::arg("value_factor"))

        .def("set_color_inversion", [](Filters::ColorMatrix& colorMatrix, const py::tuple& hue, float saturation, float value){
                colorMatrix.setColorInversion(Python::toColor3f(hue), saturation, value);
            },
            "Resets the current transformation to a fancy color inversion mode with a fixed hue point",
            py::arg("preserved_hue"), py::arg("saturation_factor"), py::arg("value_factor"))

        .def("apply_contrast", &Filters::ColorMatrix::applyContrast,
            "Applies a contrast adjustment by a given factor on top of the current transformation",
            py::arg("factor"))

        .def("set_brightness", &Filters::ColorMatrix::setBrightness,
            "Sets a brightness adjustment by a given factor (non-cumulative with respect to the current transformation)",
            py::arg("brightness"));

    /**
     * Filters::Sepia
     */
    py::class_<Filters::Sepia, Filters::PixelwiseFilter>(filters, "Sepia", "Sepia filter: an example of :class:`~beatmup.filters.PixelwiseFilter` implementation.")
        .def(py::init<>());

    /**
     * IntegerCountour2D
     */
    py::class_<IntegerContour2D>(module, "IntegerContour2D",
            "A sequence of integer-valued 2D points")

        .def(py::init<>())

        .def("add_point", &IntegerContour2D::addPoint,
            "Adds a new point to the end of the contour. Some points may be skipped to optimize the storage.",
            py::arg("x"), py::arg("y"))

        .def("clear", &IntegerContour2D::clear,
            "Removes contour content")

        .def("get_point_count", &IntegerContour2D::getPointCount,
            "Returns number of points in the contour")

        .def("get_length", &IntegerContour2D::getLength,
            "Returns contour length")

        .def("get_point",
            [](IntegerContour2D& contour, int index) { return Python::toTuple(contour.getPoint(index)); },
            py::arg("index"),
            "Returns a point by its index");

    /**
     * FloodFill
     */
    py::class_<FloodFill, AbstractTask> floodFill(module, "FloodFill",
        R"doc(
            Discovers areas of similar colors up to a tolerance threshold around given positions (seeds) in the input image.
            These areas are filled with white color in another image (output). If the output bitmap is a binary mask,
            corresponding pixels are set to `1`. The rest of the output image remains unchanged.
            Optionally, computes contours around the discovered areas and stores the contour positions.
            Also optionally, applies post-processing by dilating or eroding the discovered regions in the output image.
        )doc");

    py::enum_<FloodFill::BorderMorphology>(floodFill, "BorderMorphology",
        "Morphological postprocessing operation applied to the discovered connected components")
        .value("NONE",   FloodFill::BorderMorphology::NONE,   "no postprocessing")
        .value("DILATE", FloodFill::BorderMorphology::DILATE, "apply a dilatation")
        .value("ERODE",  FloodFill::BorderMorphology::ERODE,  "apply an erosion")
        .export_values();

    floodFill.def(py::init<>())

        .def_property("input",
            &FloodFill::getInput,
            py::cpp_function(&FloodFill::setInput, py::keep_alive<1, 2, 1>()),     // instance alive => bitmap alive
            "Input bitmap")

        .def_property("output",
            &FloodFill::getOutput,
            py::cpp_function(&FloodFill::setOutput, py::keep_alive<1, 2, 2>()),     // instance alive => bitmap alive
            "Output bitmap")

        .def_property("tolerance", &FloodFill::getTolerance, &FloodFill::setTolerance,
            "Intensity tolerance")

        .def("set_mask_pos",
            [](FloodFill& ff, const py::tuple& pos) { ff.setMaskPos(Python::toPoint<int>(pos)); },
            py::arg("pos"),
            "Specifies left-top corner position of the mask inside the input bitmap")

        .def("set_seeds", [](FloodFill& ff, const py::list& seeds) {
                IntPoint* pts = new IntPoint[seeds.size()];
                for (ssize_t i = 0; i < seeds.size(); ++i)
                    pts[i] = Python::toPoint<int>(seeds[i]);
                ff.setSeeds(pts, seeds.size());
                delete[] pts;
            },
            py::arg("seeds"),
            "Specifies a set of seeds (starting points)")

        .def("set_compute_contours", &FloodFill::setComputeContours, py::arg("compute"),
            "Enables or disables contours computation")

        .def("set_border_postprocessing", &FloodFill::setBorderPostprocessing,
            py::arg("operation"), py::arg("hold_radius"), py::arg("release_radius"),
            R"doc(
                Specifies a morphological operation to apply to the mask border.

                :param operation:      a postprocessing operation
                :param hold_radius:    erosion/dilation hold radius (output values set to 1)
                :param release_radius: erosion/dilation radius of transition from 1 to 0
            )doc")

        .def("get_bounds",
            [](FloodFill& ff, const py::tuple& pos) { return Python::toTuple(ff.getBounds()); },
            "Returns bounding box of the computed mask")

        .def("get_contour_count", &FloodFill::getContourCount,
            "Returns number of discovered contours")

        .def("get_contour", &FloodFill::getContour,
            py::arg("index"),
            "Returns a contour by index if compute_contours was set to True, throws an exception otherwise");

    /**
     * AffineMapping
     */
    py::class_<AffineMapping>(module, "AffineMapping", "2x3 affine mapping containing a 2x2 matrix and a 2D point")

        .def(py::init<>())

        .def("get_position",
            [](const AffineMapping& mapping) { return Python::toTuple(mapping.getPosition()); },
            "Returns the mapping origin")

        .def("get_matrix",
            [](const AffineMapping& mapping) { return Python::toTuple(mapping.getMatrix()); },
            "Returns the mapping matrix")

        .def("__call__",
            [](const AffineMapping& mapping, const py::tuple& point) { return Python::toTuple(mapping(Python::toPoint<float>(point))); },
            py::arg("point"),
            "Maps a point")

        .def("invert", &AffineMapping::invert,
            "Inverts the mapping")

        .def("get_inverse", (AffineMapping (AffineMapping::*)() const)&AffineMapping::getInverse,
            py::return_value_policy::take_ownership,
            "Returns inverse mapping")

        .def("get_inverse",
            [](const AffineMapping& mapping, const py::tuple& point) { return Python::toTuple(mapping.getInverse(Python::toPoint<float>(point))); },
            py::arg("point"),
            "Computes inverse mapping of a point")

        .def("set_center_position", [](AffineMapping& mapping, const py::tuple& point) {
                mapping.setCenterPosition(Python::toPoint<float>(point));
            },
            py::arg("point"),
            "Adjusts the mapping origin so that the center of the axes box matches a given point")

        .def("translate", [](AffineMapping& mapping, const py::tuple& shift) {
                mapping.translate(Python::toPoint<float>(shift));
            },
            py::arg("shift"),
            "Translates the mapping")

        .def("scale", [](AffineMapping& mapping, float factor, const py::tuple& fixedPoint) {
                mapping.scale(factor, Python::toPoint<float>(fixedPoint));
            },
            py::arg("factor"), py::arg("fixed_point") = py::make_tuple(0.0f, 0.0f),
            "Scales the mapping around a given point in target domain")

        .def("rotate_degrees", [](AffineMapping& mapping, float angle, const py::tuple& fixedPoint) {
                mapping.rotateDegrees(angle, Python::toPoint<float>(fixedPoint));
            },
            py::arg("angle"), py::arg("fixed_point") = py::make_tuple(0.0f, 0.0f),
            "Rotates the mapping around a given point in target domain")

        .def("is_point_inside", [](AffineMapping& mapping, const py::tuple& point) {
                return mapping.isPointInside(Python::toPoint<float>(point));
            },
            py::arg("point"),
            "Tests whether a point from the output domain is inside the input axes span");

    /**
     * GL::VariablesBundle
     */
    py::class_<GL::VariablesBundle>(gl, "VariablesBundle",
        "Collection storing GLSL program parameters (scalars, matrices, vectors) to communicate them from user to GPU-managing thread")

        .def("set_integer", (void (GL::VariablesBundle::*)(std::string, int))&GL::VariablesBundle::setInteger,
            py::arg("name"), py::arg("value"),
            "Sets a scalar integer uniform value")

        .def("set_integer", (void (GL::VariablesBundle::*)(std::string, int, int))&GL::VariablesBundle::setInteger,
            py::arg("name"), py::arg("x"), py::arg("y"),
            "Sets a 2D integer uniform vector value")

        .def("set_integer", (void (GL::VariablesBundle::*)(std::string, int, int, int))&GL::VariablesBundle::setInteger,
            py::arg("name"), py::arg("x"), py::arg("y"), py::arg("z"),
            "Sets a 3D integer uniform vector value")

        .def("set_integer", (void (GL::VariablesBundle::*)(std::string, int, int, int, int))&GL::VariablesBundle::setInteger,
            py::arg("name"), py::arg("x"), py::arg("y"), py::arg("z"), py::arg("w"),
            "Sets a 4D integer uniform vector value")

        .def("set_float", (void (GL::VariablesBundle::*)(std::string, float))&GL::VariablesBundle::setFloat,
            py::arg("name"), py::arg("value"),
            "Sets a scalar float uniform value")

        .def("set_float", (void (GL::VariablesBundle::*)(std::string, float, float))&GL::VariablesBundle::setFloat,
            py::arg("name"), py::arg("x"), py::arg("y"),
            "Sets a 2D float uniform vector value")

        .def("set_float", (void (GL::VariablesBundle::*)(std::string, float, float, float))&GL::VariablesBundle::setFloat,
            py::arg("name"), py::arg("x"), py::arg("y"), py::arg("z"),
            "Sets a 3D float uniform vector value")

        .def("set_float", (void (GL::VariablesBundle::*)(std::string, float, float, float, float))&GL::VariablesBundle::setFloat,
            py::arg("name"), py::arg("x"), py::arg("y"), py::arg("z"), py::arg("w"),
            "Sets a 4D float uniform vector value")

        .def("set_float_matrix2", [](GL::VariablesBundle& instance, const char* name, const std::vector<float>& matrix) {
                if (matrix.size() != 2*2)
                    throw std::invalid_argument("Expected a list-like input containing " + std::to_string(2*2) +
                            " values but got " + std::to_string(matrix.size()));
                instance.setFloatMatrix2(name, matrix.data());
            },
            py::arg("name"), py::arg("matrix"),
            "Sets a float 2*2 matrix variable value")

        .def("set_float_matrix3", [](GL::VariablesBundle& instance, const char* name, const std::vector<float>& matrix) {
                if (matrix.size() != 3*3)
                    throw std::invalid_argument("Expected a list-like input containing " + std::to_string(3*3) +
                            " values but got " + std::to_string(matrix.size()));
                instance.setFloatMatrix3(name, matrix.data());
            },
            py::arg("name"), py::arg("matrix"),
            "Sets a float 3*3 matrix variable value")

        .def("set_float_matrix4", [](GL::VariablesBundle& instance, const char* name, const std::vector<float>& matrix) {
                if (matrix.size() != 4*4)
                    throw std::invalid_argument("Expected a list-like input containing " + std::to_string(4*4) +
                            " values but got " + std::to_string(matrix.size()));
                instance.setFloatMatrix4(name, matrix.data());
            },
            py::arg("name"), py::arg("matrix"),
            "Sets a float 4*4 matrix variable value")

        .def("set_float_array", &GL::VariablesBundle::setFloatArray,
            py::arg("name"), py::arg("values"),
            "Sets a float array variable value");

    /**
     *  Metric
     */
    py::class_<Metric, AbstractTask> metric(module, "Metric", "Measures the difference between two bitmaps");

    py::enum_<Metric::Norm>(metric, "Norm", "Norm (distance) to measure between two images")
        .value("L1",    Metric::Norm::L1, "sum of absolute differences")
        .value("L2",    Metric::Norm::L2, "Euclidean distance: square root of squared differences")
        .export_values();

    metric.def(py::init<>())

        .def("set_bitmaps", (void (Metric::*)(AbstractBitmap*, AbstractBitmap*))&Metric::setBitmaps,
            py::arg("bitmap1"), py::arg("bitmap2"),
            py::keep_alive<1, 2>(), py::keep_alive<1, 3>(), // metric alive => bitmaps alive
            "Sets input images")

        .def("set_bitmaps", [](Metric& metric, AbstractBitmap* bitmap1, const py::tuple& roi1, AbstractBitmap* bitmap2, const py::tuple& roi2){
                metric.setBitmaps(bitmap1, Python::toRectangle<int>(roi1),
                                  bitmap2, Python::toRectangle<int>(roi2));
            },
            py::arg("bitmap1"), py::arg("roi1"), py::arg("bitmap2"), py::arg("roi2"),
            py::keep_alive<1, 2>(), py::keep_alive<1, 4>(), // metric alive => bitmaps alive
            "Sets input images and rectangular regions delimiting the measurement areas")

        .def("set_norm", &Metric::setNorm, "Specifies the norm to use in the measurement")

        .def("get_result", &Metric::getResult, "Returns the measurement result (after the task is executed")

        .def_static("psnr", &Metric::psnr, py::arg("bitmap1"), py::arg("bitmap2"),
            "Computes peak signal-to-noise ratio in dB for two given images");

    /**
     * ImageShader
     */
    py::class_<ImageShader, GL::VariablesBundle>(module, "ImageShader", "A GLSL program to process images")

        .def(py::init<Context&>(), py::arg("context"), py::keep_alive<1, 2>())

        .def("set_source_code", &ImageShader::setSourceCode,
            py::arg("glsl"),
            R"doc(Passes new source code to the fragment shader.
            The new source code will be compiled and linked when next rendering occurs.)doc")

        .def_property_readonly_static("INPUT_IMAGE_DECL_TYPE",
            [](py::object) { return ImageShader::INPUT_IMAGE_DECL_TYPE; },
            "A virtual input image type defined at shader compile time by ordinary texture or OES texture sampler depending on the input bound")

        .def_property_readonly_static("INPUT_IMAGE_ID",
            [](py::object) { return ImageShader::INPUT_IMAGE_ID; },
            "Shader variable name referring to the input image")

        .def_property_readonly_static("CODE_HEADER",
            [](py::object) { return ImageShader::CODE_HEADER; },
            "Shader code header containing necessary declarations");

    /**
     * ShaderApplicator
     */
    py::class_<ShaderApplicator, AbstractTask>(module, "ShaderApplicator", "A task applying an image shader to bitmaps")

        .def(py::init<>())

        .def("add_sampler", &ShaderApplicator::addSampler,
            py::arg("bitmap"), py::arg("uniform_name") = ImageShader::INPUT_IMAGE_ID,
            py::keep_alive<1, 2>(),     // applicator alive => bitmap alive
            R"doc(
                Connects a bitmap to a shader uniform variable.
                The bitmap connected to ImageShader::INPUT_IMAGE_ID is used to resolve the sampler type (ImageShader::INPUT_IMAGE_DECL_TYPE).
            )doc")

        .def("remove_sampler", &ShaderApplicator::removeSampler,
            py::arg("uniform_name"),
            R"doc(
                Removes a sampler with a uniform variable name.
                Returns True if a sampler associated to the given variable existed and was removed, false otherwise.
            )doc")

        .def("clear_samplers", &ShaderApplicator::clearSamplers, "Clears all connections of bitmaps to samplers")

        .def_property("shader",
            &ShaderApplicator::getShader,
            py::cpp_function(&ShaderApplicator::setShader, py::keep_alive<1, 2, 1>()),   // applicator alive => shader alive
            "Shader to apply to the bitmap(s)")

        .def_property("output_bitmap",
            &ShaderApplicator::getOutputBitmap,
            py::cpp_function(&ShaderApplicator::setOutputBitmap, py::keep_alive<1, 2, 2>()),   // applicator alive => bitmap alive
            "Output bitmap");

    /**
     * Scene and its layers
     */
    py::class_<Scene> scene(module, "Scene", "An ordered set of layers representing renderable content");

    py::class_<Scene::Layer> layer(scene, "Layer",
        R"doc(
            Abstract scene layer having name, type, geometry and some content to display.
            The layer geometry is defined by an AffineMapping describing the position and the orientation of the layer content in the rendered image.
        )doc");

    py::enum_<Scene::Layer::Type>(layer, "Type", "Layer type")
        .value("SCENE",         Scene::Layer::Type::SceneLayer,        "layer containing a scene")
        .value("BITMAP",        Scene::Layer::Type::BitmapLayer,       "layer displaying a bitmap")
        .value("MASKED_BITMAP", Scene::Layer::Type::MaskedBitmapLayer, "layer displaying a bitmap with mask")
        .value("SHAPED_BITMAP", Scene::Layer::Type::ShapedBitmapLayer, "layer displaying a bitmap within a shape")
        .value("SHADED_BITMAP", Scene::Layer::Type::ShadedBitmapLayer, "layer displaying a bitmap through a custom fragment shader")
        .export_values();

    layer.def("get_type", &Scene::Layer::getType, "Returns layer type")

        .def_property("name", &Scene::Layer::getName, &Scene::Layer::setName)

        .def_property("mapping", (AffineMapping& (Scene::Layer::*)())&Scene::Layer::getMapping, &Scene::Layer::setMapping,
            "Layer mapping in parent coordinates")

        .def("test_point", &Scene::Layer::testPoint,
            py::arg("x"), py::arg("y"),
            "Tests if a given point falls in the layer")

        .def("test_point", [](const Scene::Layer& layer, const py::tuple& point) {
                Point pt = Python::toPoint<float>(point);
                return layer.testPoint(pt.x, pt.y);
            },
            py::arg("point"),
            "Tests if a given point falls in the layer")

        .def("get_child", &Scene::Layer::getChild,
            py::arg("x"), py::arg("y"), py::arg("recursion_depth") = 0,
            "Picks a child layer at given point, if any")

        .def("get_child", [](const Scene::Layer& layer, const py::tuple& point, unsigned int recursionDepth) {
                Point pt = Python::toPoint<float>(point);
                return layer.getChild(pt.x, pt.y, recursionDepth);
            },
            py::arg("point"), py::arg("recursion_depth") = 0,
            "Picks a child layer at given point, if any")

        .def_property("visible", &Scene::Layer::isVisible, &Scene::Layer::setVisible,
            "Controls the layer visibility. If set to `False`, the layer and its sublayers are ignored when rendering.")

        .def_property("phantom", &Scene::Layer::isPhantom, &Scene::Layer::setPhantom,
            "If set to `True`, the layer goes \"phantom\": it and its sublayers, if any, are ignored when searching a layer by point.");

    py::class_<Scene::SceneLayer, Scene::Layer>(scene, "SceneLayer",
        "Layer containing an entire scene")
        .def("get_scene", &Scene::SceneLayer::getScene, "Returns a Scene contained in the Layer");

    py::class_<Scene::BitmapLayer, Scene::Layer>(scene, "BitmapLayer",
        R"doc(
            Layer having an image to render.
            The image has a position and orientation with respect to the layer. This is expressed with an affine mapping applied on top of the layer
            mapping.
        )doc")

        .def_property("bitmap",
            &Scene::BitmapLayer::getBitmap,
            py::cpp_function(&Scene::BitmapLayer::setBitmap, py::keep_alive<1, 2, 1>()),   // layer alive => bitmap alive
            "Bitmap attached to the layer")

        .def_property("bitmap_mapping",
        (AffineMapping& (Scene::BitmapLayer::*)())&Scene::BitmapLayer::getBitmapMapping, &Scene::BitmapLayer::setBitmapMapping,
            "Bitmap geometry mapping applied on top of the layer mapping")

        .def_property("modulation_color",
            [](Scene::BitmapLayer& layer) { return Python::toTuple(layer.getModulationColor()); },
            [](Scene::BitmapLayer& layer, const py::tuple& color){ layer.setModulationColor(Python::toColor4i(color)); },
            "Modulation color (R, G, B, A). Multiplies bitmap pixel colors when rendering");

    py::class_<Scene::CustomMaskedBitmapLayer, Scene::BitmapLayer>(scene, "CustomMaskedBitmapLayer",
            R"doc(
                Layer containing a bitmap and a mask applied to the bitmap when rendering.
                Both bitmap and mask have their own positions and orientations relative to the layer's position and orientation.
            )doc")

        .def_property("mask_mapping",
            (AffineMapping& (Scene::CustomMaskedBitmapLayer::*)())&Scene::CustomMaskedBitmapLayer::getMaskMapping,
            &Scene::CustomMaskedBitmapLayer::setMaskMapping,
            "Mask geometry mapping applied on top of the layer mapping")

        .def_property("background_color",
            [](Scene::CustomMaskedBitmapLayer& layer) { return Python::toTuple(layer.getBackgroundColor()); },
            [](Scene::CustomMaskedBitmapLayer& layer, const py::tuple& color){ layer.setBackgroundColor(Python::toColor4i(color)); },
            "Background color (R, G, B, A). Fills layer pixels falling out of the mask area");

    py::class_<Scene::MaskedBitmapLayer, Scene::CustomMaskedBitmapLayer>(scene, "MaskedBitmapLayer",
            "Bitmap layer using another bitmap as a mask")

        .def_property("mask",
            &Scene::MaskedBitmapLayer::getMask,
            py::cpp_function(&Scene::MaskedBitmapLayer::setMask, py::keep_alive<1, 2, 2>()),   // layer alive => bitmap alive
            "Mask bitmap");

    py::class_<Scene::ShapedBitmapLayer, Scene::CustomMaskedBitmapLayer>(scene, "ShapedBitmapLayer",
            "Layer containing a bitmap and a parametric mask (shape)")

        .def_property("border_width", &Scene::ShapedBitmapLayer::getBorderWidth, &Scene::ShapedBitmapLayer::setBorderWidth,
            "Mask border thickness in pixels or normalized coordinates. " \
            "These pixels are cropped out from the image and replaced with the background color.")

        .def_property("slope_width", &Scene::ShapedBitmapLayer::getSlopeWidth, &Scene::ShapedBitmapLayer::setSlopeWidth,
            "Mask border slope width in pixels or normalized coordinates. "\
            "The border slope is a linear transition from background color to image pixels.")

        .def_property("corner_radius", &Scene::ShapedBitmapLayer::getCornerRadius, &Scene::ShapedBitmapLayer::setCornerRadius,
            "Radius of mask corners in pixels or normalized coordinates")

        .def_property("in_pixels", &Scene::ShapedBitmapLayer::getInPixels, &Scene::ShapedBitmapLayer::setInPixels,
            "If set to `True`, all the parameter values are interpreted as if given in pixels. Otherwise the normalized coordinates are used.");

    py::class_<Scene::ShadedBitmapLayer, Scene::BitmapLayer>(scene, "ShadedBitmapLayer", "Bitmap layer using a custom shader")

        .def_property("shader",
            &Scene::ShadedBitmapLayer::getShader,
            py::cpp_function(&Scene::ShadedBitmapLayer::setShader, py::keep_alive<1, 2, 3>()),   // layer alive => bitmap alive
            "Fragment shader taking the layer bitmap as texture");

    scene.def(py::init<>())

        .def("new_bitmap_layer", (Scene::BitmapLayer& (Scene::*)(const char*))&Scene::newBitmapLayer,
            py::arg("name"),
            py::return_value_policy::reference, py::keep_alive<1, 0>(),     // scene alive => layer alive
            "Creates a new bitmap layer")

        .def("new_bitmap_layer", (Scene::BitmapLayer& (Scene::*)())&Scene::newBitmapLayer,
            py::return_value_policy::reference, py::keep_alive<1, 0>(),     // scene alive => layer alive
            "Creates a new bitmap layer")

        .def("new_masked_bitmap_layer", (Scene::MaskedBitmapLayer& (Scene::*)(const char*))&Scene::newMaskedBitmapLayer,
            py::arg("name"),
            py::return_value_policy::reference, py::keep_alive<1, 0>(),     // scene alive => layer alive
            "Creates a new masked bitmap layer")

        .def("new_masked_bitmap_layer", (Scene::MaskedBitmapLayer& (Scene::*)())&Scene::newMaskedBitmapLayer,
            py::return_value_policy::reference, py::keep_alive<1, 0>(),     // scene alive => layer alive
            "Creates a new masked bitmap layer")

        .def("new_shaped_bitmap_layer", (Scene::ShapedBitmapLayer& (Scene::*)(const char*))&Scene::newShapedBitmapLayer,
            py::arg("name"),
            py::return_value_policy::reference, py::keep_alive<1, 0>(),     // scene alive => layer alive
            "Creates a new shaped bitmap layer")

        .def("new_shaped_bitmap_layer", (Scene::ShapedBitmapLayer& (Scene::*)())&Scene::newShapedBitmapLayer,
            py::return_value_policy::reference, py::keep_alive<1, 0>(),     // scene alive => layer alive
            "Creates a new shaped bitmap layer")

        .def("new_shaded_bitmap_layer", (Scene::ShadedBitmapLayer& (Scene::*)(const char*))&Scene::newShadedBitmapLayer,
            py::arg("name"),
            py::return_value_policy::reference, py::keep_alive<1, 0>(),     // scene alive => layer alive
            "Creates a new shaded bitmap layer")

        .def("new_shaded_bitmap_layer", (Scene::ShadedBitmapLayer& (Scene::*)())&Scene::newShadedBitmapLayer,
            py::return_value_policy::reference, py::keep_alive<1, 0>(),     // scene alive => layer alive
            "Creates a new shaded bitmap layer")

        .def("add_scene", &Scene::addScene,
            py::return_value_policy::reference, py::keep_alive<1, 0>(),     // scene alive => layer alive
            "Adds a subscene to the current scene.")

        .def("get_layer", (Scene::Layer* (Scene::*)(const char*) const)&Scene::getLayer,
            py::arg("name"),
            "Retrieves a layer by its name or None if not found")

        .def("get_layer", (Scene::Layer& (Scene::*)(int) const)&Scene::getLayer,
            py::arg("index"),
            "Retrieves a layer by its index")

        .def("get_layer", (Scene::Layer* (Scene::*)(float, float, unsigned int) const)&Scene::getLayer,
            py::arg("x"), py::arg("y"), py::arg("recursion_depth") = 0,
            "Retrieves a layer present at a specific point of the scene or None if not found")

        .def("get_layer_index", &Scene::getLayerIndex,
            py::arg("layer"),
            "Retrieves layer index in the scene or -1 if not found")

        .def("get_layer_count", &Scene::getLayerCount, "Returns total number of layers in the scene");

    /**
     * SceneRenderer
     */
    py::class_<SceneRenderer, AbstractTask> sceneRenderer(module, "SceneRenderer",
        R"doc(
            AbstractTask rendering a Scene.
            The rendering may be done to a given bitmap or on screen, if the platform supports on-screen rendering.
        )doc");

    py::enum_<SceneRenderer::OutputMapping>(sceneRenderer, "OutputMapping", "Scene coordinates to output (screen or bitmap) pixel coordinates mapping")
        .value("STRETCH",          SceneRenderer::OutputMapping::STRETCH,          "output viewport covers entirely the scene axis span, aspect ratio is not preserved in general")
        .value("FIT_WIDTH_TO_TOP", SceneRenderer::OutputMapping::FIT_WIDTH_TO_TOP, "width is covered entirely, height is resized to keep aspect ratio, the top borders are aligned")
        .value("FIT_WIDTH",        SceneRenderer::OutputMapping::FIT_WIDTH,        "width is covered entirely, height is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center")
        .value("FIT_HEIGHT",       SceneRenderer::OutputMapping::FIT_HEIGHT,       "height is covered entirely, width is resized to keep aspect ratio, point (0.5, 0.5) is mapped to the output center")
        .export_values();

    sceneRenderer.def(py::init<>())

        .def_property("output",
            &SceneRenderer::getOutput,
            py::cpp_function(&SceneRenderer::setOutput, py::keep_alive<1, 2, 1>()),     // instance alive => bitmap alive
            "Output bitmap")

        .def_property("scene",
            &SceneRenderer::getScene,
            py::cpp_function(&SceneRenderer::setScene, py::keep_alive<1, 2, 2>()),      // instance alive => scene alive
            "Scene")

        .def_property("output_mapping", &SceneRenderer::getOutputMapping, &SceneRenderer::setOutputMapping,
            "Specifies how the scene coordinates [0,1]² are mapped to the output (screen or bitmap) pixel coordinates.")

        .def_property("output_reference_width", &SceneRenderer::getOutputReferenceWidth, &SceneRenderer::setOutputReferenceWidth,
            "Value overriding output width for elements that have their size in pixels, in order to render a resolution-independent picture")

        .def_property("output_pixels_fetching", &SceneRenderer::getOutputPixelsFetching, &SceneRenderer::setOutputPixelsFetching,
            R"doc(
                If set to `True`, the output image data is pulled from GPU to CPU memory every time the rendering is done.
                This is convenient if the rendered image is an application output result, and is further stored or sent through the network.
                Otherwise, if the image is to be further processed inside Beatmup, the pixel transfer likely introduces an unnecessary latency and may
                cause FPS drop in real-time rendering.
                Has no effect in on-screen rendering.
            )doc")

        .def_property("background_image",
            &SceneRenderer::getBackgroundImage,
            py::cpp_function(&SceneRenderer::setBackgroundImage, py::keep_alive<1, 2, 3>()),    // instance alive => bitmap alive
            "Image to pave the background.")

        .def("reset_output", &SceneRenderer::resetOutput,
            R"doc(
                Removes a bitmap from the renderer output, if any, and switches to on-screen rendering.
                The rendering is done on the display currently connected to the Context running the rendering task.
            )doc")

        .def("pick_layer", &SceneRenderer::pickLayer,
            py::arg("x"), py::arg("y"), py::arg("inPixels"), R"doc(
                Searches for a layer at a given position.
                In contrast to :func:`~beatmup.Scene.get_layer` it takes into account the output mapping.

                :param x:       x coordinate.
                :param y:       y coordinate.
                :param pixels:  If `True`, the coordinates are taken in pixels.

                Returns the topmost layer at the given position if any, None if no layer found.
            )doc");

    /**
     * CustomPipeline::TaskHolder
     */
    py::class_<CustomPipeline, AbstractTask> customPipeline(module, "CustomPipeline",
        R"doc(
            Custom pipeline: a sequence of tasks to be executed as a whole.
            Acts as an AbstractTask. Built by adding tasks one by one and calling measure() at the end.
        )doc");

    py::class_<CustomPipeline::TaskHolder>(customPipeline, "TaskHolder",
            "A task within a pipeline")

        .def("get_task", &CustomPipeline::TaskHolder::getTask,
            "Returns the task in the current holder")

        .def("get_run_time", &CustomPipeline::TaskHolder::getRunTime,
            "Returns last execution time in milliseconds");

    /**
     * CustomPipeline
     */
    customPipeline
        .def("get_task_count", &CustomPipeline::getTaskCount,
            "Returns number of tasks in the pipeline")

        .def("get_task", &CustomPipeline::getTask, py::arg("index"),
            py::return_value_policy::reference,
            "Retrieves a task by its index")

        .def("get_task_index", &CustomPipeline::getTaskIndex, py::arg("holder"),
            "Retrieves task index if it is in the pipeline; returns -1 otherwise")

        .def("add_task", &CustomPipeline::addTask, py::arg("task"),
            py::keep_alive<1, 2>(),      // pipeline alive => task alive
            py::return_value_policy::reference,
            "Adds a new task to the end of the pipeline")

        .def("insert_task", &CustomPipeline::insertTask, py::arg("task"), py::arg("before"),
            py::keep_alive<1, 2>(),      // pipeline alive => task alive
            py::return_value_policy::reference,
            "Inserts a task in a specified position of the pipeline before another task")

        .def("remove_task", &CustomPipeline::removeTask, py::arg("task"),
            "Removes a task from the pipeline, if any. Returns True on success")

        .def("measure", &CustomPipeline::measure,
            "Determines pipeline execution mode and required thread count");

    /**
     * Multitask
     */
    py::class_<Multitask, CustomPipeline> multitask(module, "Multitask",
        R"doc(
            Conditional multiple tasks execution.

            Beatmup offers a number of tools allowing to pipeline several tasks into a single one. This technique is particularly useful for designing
            complex multi-stage image processing pipelines.

            Multitask is the simplest such tool. It allows to concatenate different tasks into a linear conveyor and run them all or selectively.
            To handle this selection, each task is associated with a repetition policy specifying the conditions whether this given task is executed
            or ignored when the pipeline is running.

            Specifically, there are two extreme modes that force the task execution every time (REPEAT_ALWAYS) or its unconditional skipping
            (IGNORE_ALWAYS) and two more sophisticated modes with the following behavior:

             - IGNORE_IF_UPTODATE skips the task if no tasks were executed among the ones coming before the current task in the pipeline;
             - REPEAT_UPDATE forces task repetition one time on next run and just after switches the repetition policy to IGNORE_IF_UPTODATE.
        )doc");

    py::enum_<Multitask::RepetitionPolicy>(multitask, "RepetitionPolicy",
        "Determines when a specific task in the sequence is run when the whole sequence is invoked")
        .value("REPEAT_ALWAYS",         Multitask::RepetitionPolicy::REPEAT_ALWAYS,         "execute the task unconditionally on each run")
        .value("REPEAT_UPDATE",         Multitask::RepetitionPolicy::REPEAT_UPDATE,         "execute the task one time then switch to IGNORE_IF_UPTODATE")
        .value("IGNORE_IF_UPTODATE",    Multitask::RepetitionPolicy::IGNORE_IF_UPTODATE,    "do not execute the task if no preceding tasks are run")
        .value("IGNORE_ALWAYS",         Multitask::RepetitionPolicy::IGNORE_ALWAYS,         "do not execute the task")
        .export_values();

    multitask
        .def(py::init<>())

        .def("get_repetition_policy", &Multitask::getRepetitionPolicy, py::arg("task"),
            "Returns repetition policy of a specific task in the pipeline.")

        .def("set_repetition_policy", &Multitask::setRepetitionPolicy, py::arg("task"), py::arg("policy"),
            R"doc(
                Sets repetition policy of a task. If the pipeline is processing at the moment of the call, it is the application responsibility to abort
                and restart it, if the policy change needs to be applied immediately.
            )doc");

    /**
     * ChunkCollection
     */
    py::class_<ChunkCollection>(module, "ChunkCollection",
        R"doc(
            A key-value pair set storing pieces of arbitrary data (chunks) under string keys.
            A chunk is a header and a piece of data packed in memory like this: (idLength[4], id[idLength], size[sizeof(chunksize_t)], data[size])
            ChunkCollection defines an interface to retrieve chunks by their ids.
        )doc")
        .def("open", &ChunkCollection::open, "Opens the collection to read chunks from it.")
        .def("close", &ChunkCollection::close, "Closes the collection after a reading session.")
        .def("size", &ChunkCollection::size, "Returns the number of chunks available in the collection after it is opened.")
        .def("chunk_exists", &ChunkCollection::chunkExists, py::arg("id"),
            R"doc(
                Check if a specific chunk exists.

                :param id:    the chunk id

                Returns `True` if only the chunk exists in the collection.
            )doc")
        .def("chunk_size", &ChunkCollection::chunkSize, py::arg("id"),
            R"doc(
                Retrieves size of a specific chunk.

                :param id:    the chunk id

                Return size of the chunk in bytes, 0 if not found.
            )doc")
        .def("__getitem__", [](ChunkCollection& collection, const std::string& id) -> py::object {
            if (collection.chunkExists(id)) {
                Chunk chunk(collection, id);
                return py::bytes(static_cast<const char*>(chunk()), chunk.size());
            }
            return py::none();
        }, py::arg("id"), "Returns the chunk data by its id");

    /**
     * ChunkFile
     */
    py::class_<ChunkFile, ChunkCollection>(module, "ChunkFile",
        R"doc(
            File containing chunks.
            The file is not loaded in memory, but is scanned when first opened to collect the information about available chunks.
        )doc")
        .def(py::init<const std::string&, bool>(), py::arg("filename"), py::arg("open_now") = true, R"doc(
            Creates a chunkfile accessor.
            The file content is not read until open() is called.

            :param filename:  the file name / path
            :param open_now:  if `true`, the file is read right away. Otherwise it is done on open() call.
                        No information is available about chunks in the file until it is opened.
        )doc");

    /**
     * Python::WritableChunkCollection
     */
    py::class_<Python::WritableChunkCollection, ChunkCollection>(module, "WritableChunkCollection",
        R"doc(
            Writable ChunkCollection implementation for Python.
            Allows to exchange binary data without copying.
        )doc")
        .def(py::init<>())

        .def("__setitem__", [](Python::WritableChunkCollection& collection, const std::string& id, py::buffer& buffer) { collection[id] = buffer; },
            "Stores new chunk")

        .def("__getitem__", [](Python::WritableChunkCollection& collection, const std::string& id) -> py::object {
            if (collection.chunkExists(id))
                return collection[id];
            return py::none();
        }, py::arg("id"), "Returns the chunk data by its id")

        .def("save", &Python::WritableChunkCollection::save, py::arg("filename"), py::arg("append"), R"doc(
            Saves the collection to a file.

            :param filename:     The name of the file to write chunks to
            :param append:       If True, writing to the end of the file (keeping the existing content). Rewriting the file otherwise.
        )doc");

    /**
     * NNets::ActivationFunction
     */
    py::enum_<NNets::ActivationFunction>(nnets, "ActivationFunction", "Activation function specification")
        .value("DEFAULT",      NNets::ActivationFunction::DEFAULT,      "default activation: 0..1 bounded ReLU (identity clipped to 0..1 range)")
        .value("BRELU6",       NNets::ActivationFunction::BRELU6,       "0.167 times identity clipped to 0..1 range")
        .value("SIGMOID_LIKE", NNets::ActivationFunction::SIGMOID_LIKE, "a piecewise-linear sigmoid function approximation")
        .export_values();

    /**
     * NNets::Size::Padding
     */
    py::enum_<NNets::Size::Padding>(nnets, "Padding", "Zero padding specification")
        .value("SAME",  NNets::Size::Padding::SAME,  "operation output size matches its input size for unit strides")
        .value("VALID", NNets::Size::Padding::VALID, "no zero padding")
        .export_values();

    /**
     * NNets::AbstractOperation
     */
    py::class_<NNets::AbstractOperation>(nnets, "AbstractOperation",
        R"doc(
            Abstract neural net operation (layer).
            Has a name used to refer the operation in a Model. The operation data (such as convolution weights) is provided through a ChunkCollection
            in single precision floating point format, where the chunks are searched by operation name.
            Operations have several inputs and outputs numbered starting from zero.
        )doc")
        .def_property_readonly("name", &NNets::AbstractOperation::getName,
            "Operation name")

        .def_property_readonly("input_count", &NNets::AbstractOperation::getInputCount,
            "Number of operation inputs")

        .def_property_readonly("output_count", &NNets::AbstractOperation::getOutputCount,
            "Number of operation outputs");

    /**
     * NNets::Conv2D
     */
    py::class_<NNets::Conv2D, NNets::AbstractOperation>(nnets, "Conv2D",
        R"doc(
            2D convolution operation computed on GPU.
            Has 2 inputs: main and residual (detailed below), and a single output.
            Constraints:

                * Input and output are 3D tensors with values in [0, 1] range sampled over 8 bits.
                * Number of input feature maps is 3 or a multiple of 4.
                * Number of output feature maps is a multiple of 4.
                * For group convolutions, each group contains a multiple of 4 input channels and a multiple of 4 output
                  channels, or exactly 1 input and 1 output channel (i.e., depthwise).
                * Kernels are of square shape.
                * Strides are equal along X and Y.
                * Dilations are equal to 1.
                * If an image is given on input (3 input feature maps), only valid padding is supported.
                * An activation function is always applied on output.

            Raspberry Pi-related constraints:

                * Pi cannot sample more than 256 channels to compute a single output value. Actual practical limit is
                  yet lower: something about 128 channels for pointwise convolutions and less than 100 channels for
                  bigger kernels. When the limit is reached, Pi OpenGL driver reports an out of memory error (0x505).

            Features:

                * Bias addition integrated.
                * An optional residual input is available: a tensor of output shape added to the convolution result
                  before applying the activation function.
        )doc")

        .def(py::init<const std::string&, const int, const int, const int, const int, const NNets::Size::Padding, const bool, const int, const NNets::ActivationFunction>(),
            py::arg("name"), py::arg("kernel_size"), py::arg("num_input_channels"), py::arg("num_output_channels"),
            py::arg("stride") = 1,
            py::arg("padding") = NNets::Size::Padding::VALID,
            py::arg("use_bias") = true,
            py::arg("num_groups") = 1,
            py::arg("activation") = NNets::ActivationFunction::DEFAULT,
            R"doc(
                Instantiates a 2D convolution operation.

                :param name:                  operation name.
                :param kernel_size:           convolution kernel size.
                :param num_input_channels:    number of input feature map channels (input depth).
                :param num_output_channels:   number of output feature map channels (output depth).
                :param stride:                convolution stride.
                :param padding:               padding policy.
                :param use_bias:              if `true`, the bias addition is enabled. The bias vector is searched in the model data.
                :param num_groups:            number of convolution groups to get a group/depthwise convolution.
                :param activation:            activation function applied to the operation output.
            )doc")

        .def_property_readonly("use_bias", &NNets::Conv2D::isBiasUsed, "Returns `true` if bias addition is enabled")

        .def_property_readonly_static("filters_chunk_suffix", [](py::object){ return NNets::Conv2D::FILTERS_CHUNK_SUFFIX; },
            "Suffix added to the op name to get the filters chunk id in the model data")

        .def_property_readonly_static("bias_chunk_suffix", [](py::object){ return NNets::Conv2D::BIAS_CHUNK_SUFFIX; },
            "Suffix added to the op name to get the filters chunk id in the model data");

    /**
     * NNets::Pooling2D
     */
    py::class_<NNets::Pooling2D, NNets::AbstractOperation> pooling2d(nnets, "Pooling2D",
        R"doc(
            2D pooling operation computed on GPU.
            Has a single input and a single output.
            Constraints:

                * Input and output are 3D tensors with values in [0, 1] range sampled over 8 bits.
                * Number of feature maps is a multiple of 4.
                * Pooling area is of square shape.
                * Strides are equal along X and Y.
                * Average pooling only accepts valid zero padding,

            Raspberry Pi-related constraints:

                * Pi cannot sample more than 256 channels to compute a single output value. Actual practical limit is
                  yet lower: pooling size may be limited by 10. When the limit is reached, Pi OpenGL driver reports an
                  out of memory error (0x505).
        )doc");

    /**
     * NNets::Pooling2D::Operator
     */
    py::enum_<NNets::Pooling2D::Operator>(pooling2d, "Operator", "Pooling operator specification")
        .value("MAX",       NNets::Pooling2D::Operator::MAX,     "max pooling")
        .value("AVERAGE",   NNets::Pooling2D::Operator::AVERAGE, "average pooling")
        .export_values();

    pooling2d.def(py::init<const std::string&, const NNets::Pooling2D::Operator, const int, const int, const NNets::Size::Padding>(),
        py::arg("name"), py::arg("operator"), py::arg("size"),
        py::arg("stride") = 0,
        py::arg("padding") = NNets::Size::Padding::VALID,
        R"doc(
            Instantiates a 2D pooling operation.

            :param name:          layer name.
            :param op:            pooling operator.
            :param size:          spatial pooling operational size.
            :param stride:        pooling stride; if 0, the size is used.
            :param padding:       zero padding applied to the input.
        )doc");

    /**
     * NNets::Dense
     */
    py::class_<NNets::Dense, NNets::AbstractOperation>(nnets, "Dense",
        R"doc(
            Dense (linear) layer.
            Computes `A*x + b` for input feature vector `x`, a matrix `A` and an optional bias vector `b`.
            Accepts a GL::Vector or a flat Storage view on input, amd only a GL::Vector on output.
        )doc")
        .def(py::init<Context&, const std::string&, int, bool>(),
            py::arg("context"), py::arg("name"), py::arg("num_output_dims"), py::arg("use_bias"),
            py::keep_alive<1, 2>(),     // operation alive => context alive
            R"doc(
                Creates a Dense operation.

                :param context:           a context instance
                :param name:              operation name
                :param num_output_dims:   number of output dimensions
                :param use_bias:          if True, the bias vector addition is enabled
            )doc")

        .def_property_readonly_static("matrix_chunk_suffix", [](py::object){ return NNets::Dense::MATRIX_CHUNK_SUFFIX; },
            "Suffix added to the op name to get the matrix chunk id in the model data")

        .def_property_readonly_static("bias_chunk_suffix", [](py::object){ return NNets::Dense::BIAS_CHUNK_SUFFIX; },
            "Suffix added to the op name to get the bias chunk id in the model data");

    /**
     * NNets::ImageSampler
     */
    py::class_<NNets::ImageSampler, NNets::AbstractOperation>(nnets, "ImageSampler",
        R"doc(
            Image preprocessing operation.
                Samples an image of a fixed size from an arbitrary size texture. Has three key missions.
                * If enabled, performs a center crop keeping the output aspect ratio (otherwise the input is stretched to fit the output).
                * If enabled, uses linear interpolation when possible to reduce aliasing (otherwise nearest neighbor sampling is used).
                * Brings support of OES textures. This allows for example to read data directly from camera in Android.
        )doc")
        .def(py::init([](const std::string& name, const py::tuple& size, bool centerCrop, bool linearInterpolation) {
                return new NNets::ImageSampler(name, Python::toPoint<int>(size), centerCrop, linearInterpolation);
            }),
            py::arg("name"), py::arg("size"), py::arg("center_crop") = true, py::arg("linear_interp") = true,
        R"doc(
            Creates an instance of image preprocessing operation.

            :param name:          operation name
            :param size:          a tuple containing output image size in pixels
            :param center_crop:   if True, the center crop is enabled
            :param linear_interp: if True, the linear interpolation is enabled
        )doc")

        .def_property("rotation", &NNets::ImageSampler::getRotation, &NNets::ImageSampler::setRotation, "Number of times a clockwise rotation by 90 degree is applied to the input image");

    /**
     * NNets::Softmax
     */
    py::class_<NNets::Softmax, NNets::AbstractOperation>(nnets, "Softmax",
        R"doc(
            Softmax layer.
            It does not have output, but acts as a sink. The resulting probabilities are returned by getProbabilities().
            This operation is executed on CPU.
        )doc")
        .def(py::init<const std::string&>(), py::arg("name"), R"doc(
            Creates a softmax layer.

            :param name:     operation name
        )doc")

        .def("get_probabilities", &NNets::Softmax::getProbabilities, "Returns the list of probabilities");

    /**
     * NNets::Model
     */
    py::class_<NNets::Model>(nnets, "Model",
        R"doc(
            Neural net model.
            Contains a list of operations and programmatically defined interconnections between them using addConnection().
            Enables access to the model memory at any point in the model through addOutput() and getModelData().
            The memory needed to store internal data during the inference is allocated automatically; storages are reused when possible.
            The inference of a Model is performed by InferenceTask.
        )doc")

        .def(py::init<Context&>(), py::arg("context"), py::keep_alive<1, 2>())     // model alive => context alive

        .def("append", (void (NNets::Model::*)(NNets::AbstractOperation*, bool))&NNets::Model::append,
            py::arg("new_op"), py::arg("connect") = false,
            py::keep_alive<1, 2>(),    // model alive => op alive
            R"doc(
                Adds a new operation to the model.
                The operation is added to the end of the operations list. The execution order corresponds to the addition order.

                :param new_op:    the new operation
                :param connect:   if `True`, the main operation input is connected to the operation output
            )doc")

        .def("add_operation", (void (NNets::Model::*)(const std::string&, NNets::AbstractOperation*))&NNets::Model::addOperation,
            py::arg("op_name"), py::arg("new_op"),
            py::keep_alive<1, 3>(),    // model alive => op alive
            R"doc(
                Adds a new operation to the model before another operation in the execution order.
                The Model does not takes ownership of the passed pointer. The new operation is not automatically connected to other operations.

                :param op_name:   name of the operation the new operation is inserted before
                :param new_op:    the new operation
            )doc")

        .def("add_connection", (void (NNets::Model::*)(const std::string&, const std::string&, int, int, int))&NNets::Model::addConnection,
            py::arg("source_op"), py::arg("dest_op"), py::arg("output") = 0, py::arg("input") = 0, py::arg("shuffle") = 0,
            R"doc(
                Adds a connection between two given ops.

                :param source_op: name of the operation emitting the data
                :param dest_op:   name of the operation receiving the data
                :param output:    output number of the source operation
                :param input:     input number of the destination operation
                :param shuffle:   if greater than zero, the storage is shuffled.
                                  For shuffle = `n`, the output channels are sent to the destination operation in the following order:
                                  `0, 1, 2, 3, 4n, 4n+1, 4n+2, 4n+3, 8n, 8n+1, 8n+2, 8n+3, ..., 4, 5, 6, 7, 4n+4, 4n+5, 4n+6, 4n+7, 8n+4, ...`
            )doc")

        .def("add_output", (void (NNets::Model::*)(const std::string&, int))&NNets::Model::addOutput, py::arg("operation"), py::arg("output") = 0,
            R"doc(
                Enables reading output data from the model memory through get_output_data().
                A given operation output is connected to a storage that might be accessed by the application after the run.

                :operation: name of the operation to get data from
                :output:    the operation output index
            )doc")

        .def("add_output", (void (NNets::Model::*)(const NNets::AbstractOperation&, int))&NNets::Model::addOutput, py::arg("operation"), py::arg("output") = 0,
            R"doc(
                Enables reading output data from the model memory through get_output_data().
                A given operation output is connected to a storage that might be accessed by the application after the run.

                :operation: operation to get data from. If not in the model, an exception is thrown.
                :output:    the operation output index
            )doc")

        .def("get_output_data", &Python::getModelOutputDataByName, py::arg("op_name"), py::arg("output") = 0,
            R"doc(
                Reads data from the model memory.
                add_output() is needed to be called first in order to enable reading the data. Otherwise None is returned.

                :op_name:   name of the operation to get data from
                :output:    the operation output index

                Returns data array or None.
            )doc")

        .def("get_output_data", &Python::getModelOutputDataByOp, py::arg("operation"), py::arg("output") = 0,
            R"doc(
                Reads data from the model memory.
                add_output() is needed to be called first in order to enable reading the data. Otherwise None is returned.

                :operation:  the operation to get data from
                :output:     the operation output index

                Returns data array or None.
            )doc")

        .def("get_first_operation", (NNets::AbstractOperation& (NNets::Model::*)())&NNets::Model::getFirstOperation,
            py::return_value_policy::reference,
            "Returns the first operation in the model")

        .def("get_last_operation", (NNets::AbstractOperation& (NNets::Model::*)())&NNets::Model::getLastOperation,
            py::return_value_policy::reference,
            "Returns the last operation in the model")

        .def("serialize", &NNets::Model::serializeToString, "Returns serialized representation of the model as a string.")

        .def("count_multiply_adds", &NNets::Model::countMultiplyAdds, "Provides an estimation of the number of multiply-adds characterizing the model complexity.")

        .def("count_texel_fetches", &NNets::Model::countTexelFetches, "Provides an estimation of the total number of texels fetched by all the operations in the model per image.");

    /**
     * NNets::DeserializedModel
     */
    py::class_<NNets::DeserializedModel, NNets::Model>(nnets, "DeserializedModel",
        R"doc(
            Model reconstructed from a serialized representation.
            The representation format is the one rendered with Model::serialize(): a YAML-like listing containing "ops" and "connections" sections
            describing the model operations in execution order and connections between them respectively (see NNetsModelSerialization).
        )doc")

        .def(py::init<Context&, const std::string&>(), py::arg("context"), py::arg("str"),
            py::keep_alive<1, 2>()      // model alive => context alive
        );

    /**
     * NNets::InferenceTask
     */
    py::class_<NNets::InferenceTask, AbstractTask>(nnets, "InferenceTask", "Task running inference of a Model")
        .def(py::init<NNets::Model&, ChunkCollection&>(), py::arg("model"), py::arg("data"),
            py::keep_alive<1, 2>(), py::keep_alive<1, 3>())    // task alive => model and data alive

        .def("connect", (void (NNets::InferenceTask::*)(AbstractBitmap&, const std::string&, int))&NNets::InferenceTask::connect,
            py::arg("image"), py::arg("op_name"), py::arg("input_index") = 0,
            py::keep_alive<1, 2, 1>(),     // task alive => image alive
            R"doc(
                Connects an image to a specific operation input.
                Ensures the image content is up-to-date in GPU memory by the time the inference is run.

                :image:       the image
                :op_name:     the operation name
                :input_index: the input index of the operation
            )doc")

        .def("connect", (void (NNets::InferenceTask::*)(AbstractBitmap&, NNets::AbstractOperation&, int))&NNets::InferenceTask::connect,
            py::arg("image"), py::arg("operation"), py::arg("input_index") = 0,
            py::keep_alive<1, 2, 1>(),     // task alive => image alive
            R"doc(
                Connects an image to a specific operation input.
                Ensures the image content is up-to-date in GPU memory by the time the inference is run.

                :image:            The image
                :operation:        The operation
                :input_index:      The input index of the operation
            )doc");

    /**
     * NNets::Classifier
     */
    py::class_<NNets::Classifier, NNets::Model, NNets::InferenceTask>(nnets, "Classifier",
        R"doc(
            Image classifier base class.
            Makes a runnable AbstractTask from a Model. Adds an image input and a vector of probabilities for output.
        )doc")

        .def("__call__", &NNets::Classifier::operator(),
            R"doc(
                Classifies an image (blocking).
                The very first call includes the model preparation and might be slow as hell. Subsequent calls only run the inference and are likely
                much faster.

                :param input:    The input image

                Returns a vector of probabilities per class.
            )doc")

        .def("start", &NNets::Classifier::start,
            R"doc(
                Initiates the classification of a given image.
                The call is non-blocking.

                :param input:    The input image

                Returns a job corresponding to the submitted task.
            )doc")

        .def("get_probabilities", &NNets::Classifier::getProbabilities,
            "Returns the last classification results (vector of probabilities per class).");

}
#
#    Beatmup image and signal processing library
#    Copyright (C) 2020, lnstadrum
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
import beatmup, unittest

SAVE_BITMAPS = False


class BasicTests(unittest.TestCase):
    def test_context(self):
        """ Beatmup.Context API tests
        """
        ctx = beatmup.Context()

        assert not ctx.busy()
        assert not ctx.is_gpu_queried()
        assert not ctx.is_gpu_ready()

        ctx.warm_up_gpu()
        vendor, renderer = ctx.query_gpu_info()

        assert ctx.is_gpu_ready()
        assert ctx.is_gpu_queried()

        count = 3
        ctx.limit_worker_count(count)
        assert ctx.max_allowed_worker_count() == count


    def test_bitmaps(self):
        """ Bitmap and bitmaptools operations tests
        """
        ctx = beatmup.Context()
        w, h, c = 320, 240, 11
        bitmap = beatmup.bitmaptools.chessboard(ctx, w, h, c)
        ctx2 = beatmup.Context()
        copy = beatmup.bitmaptools.make_copy(bitmap, ctx2, beatmup.PixelFormat.SINGLE_BYTE)
        assert copy.get_width() == w
        assert copy.get_height() == h
        assert copy.get_pixel_format() == beatmup.PixelFormat.SINGLE_BYTE
        assert copy.get_memory_size() == w * h


    def test_affine_mapping(self):
        """ AffineMapping test
        """

        from math import sqrt
        # creation
        map = beatmup.AffineMapping()
        self.assertEqual(map.get_position(), (0, 0))
        self.assertEqual(map.get_matrix(), ((1, 0), (0, 1)))

        # rotation and scaling
        map.rotate_degrees(45)
        map.scale(1 / sqrt(2))

        # isPointInside
        self.assertEqual(map.is_point_inside((1, 0)), False)
        self.assertEqual(map.is_point_inside((0, 0.1)), True)

        # mapping a point
        testPoint = map((1, 1))
        self.assertAlmostEqual(testPoint[0], 0, places=5)
        self.assertAlmostEqual(testPoint[1], 1, places=5)

        # inversion
        inverse = map.get_inverse()
        map.invert()
        self.assertEqual(map.get_position(), inverse.get_position())
        self.assertEqual(map.get_matrix(), inverse.get_matrix())
        testPoint = map(testPoint)
        self.assertAlmostEqual(testPoint[0], 1, places=5)
        self.assertAlmostEqual(testPoint[1], 1, places=5)

        # center position
        map.set_center_position((0.123, 0.456))
        testPoint = map((0.5, 0.5))
        self.assertAlmostEqual(testPoint[0], 0.123, places=5)
        self.assertAlmostEqual(testPoint[1], 0.456, places=5)


class ResamplerTests(unittest.TestCase):
    def test_x2(self):
        """ x2 neural resampler test
        """
        ctx = beatmup.Context()
        resampler = beatmup.BitmapResampler(ctx)
        resampler.mode = beatmup.BitmapResampler.CONVNET
        input = beatmup.InternalBitmap(ctx, "../images/fecamp.bmp")
        output = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.TRIPLE_BYTE, input.get_width() * 2, input.get_height() * 2)
        resampler.input = input
        resampler.output = output
        ctx.submit_task(resampler)
        ctx.wait()
        if SAVE_BITMAPS:
            output.save_bmp("test_x2.bmp")


class FloodFillTests(unittest.TestCase):
    def test_floodfill(self):
        """ FloodFill test
        """
        ctx = beatmup.Context()

        # make bitmaps
        cell_step = 16
        input = beatmup.bitmaptools.chessboard(ctx, 320, 240, cell_step)
        output = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.BINARY_MASK, 320, 240)
        output.zero()

        # make a FloodFill instance
        ff = beatmup.FloodFill()
        ff.set_compute_contours(True)
        ff.tolerance = 0.01
        ff.input = input
        ff.output = output
        dilation = 3
        ff.set_border_postprocessing(beatmup.FloodFill.BorderMorphology.DILATE, dilation, 0)

        # plant 3 seeds, one in a middle square
        ff.set_seeds([(12,34), (123, 234), (155, 118)])

        # run
        ctx.perform_task(ff)

        # check: 3 components expected, all of the same length
        self.assertEqual(ff.get_contour_count(), 3)
        for i in range(3):
            self.assertEqual(ff.get_contour(i).get_length(), cell_step * 4 - 1)

        if SAVE_BITMAPS:
            output.save_bmp("test_floodfill.bmp")


class SceneRenderingTests(unittest.TestCase):
    def test_rendering(self):
        """ SceneRenderer test
        """
        ctx = beatmup.Context()

        # make bitmaps
        input = beatmup.bitmaptools.chessboard(ctx, 1024, 768, 32, beatmup.PixelFormat.TRIPLE_BYTE)
        output = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.TRIPLE_BYTE, 1024, 1024)

        # create shaders
        distortion = beatmup.ImageShader(ctx)
        distortion.set_source_code(distortion.CODE_HEADER + """
            highp vec2 distort(vec2 xy) {
                highp vec2 r = xy - vec2(0.5, 0.5);
                highp float t = length(r);
                return (-0.5 * t * t + 0.9) * r + vec2(0.5, 0.5);
            }
            void main() {
                gl_FragColor = texture2D(image, distort(texCoord));
            }
            """)

        gray_shift = beatmup.ImageShader(ctx)
        gray_shift.set_source_code(gray_shift.CODE_HEADER + """
            highp float gray(vec2 pos) {
                highp vec4 clr = texture2D(image, pos);
                return 0.333 * (clr.r + clr.g + clr.b);
            }
            void main() {
                gl_FragColor = vec4(
                    gray(texCoord + vec2(0.01, 0.01)),
                    gray(texCoord),
                    gray(texCoord - vec2(0.01, 0.01)),
                    1.0
                );
            }
            """)
        
        # create a scene
        scene = beatmup.Scene()
        layer = scene.new_shaped_bitmap_layer()
        layer.bitmap = input
        layer.mapping.scale(0.48)
        layer.mapping.rotate_degrees(1)
        layer.mapping.set_center_position((0.25, 0.75))
        layer.corner_radius = 0.05
        layer.slope_width = 0.01
        layer.in_pixels = False

        layer = scene.new_shaded_bitmap_layer()
        layer.bitmap = input
        layer.mapping.scale(0.48)
        layer.mapping.rotate_degrees(-1)
        layer.mapping.set_center_position((0.75, 0.25))
        layer.shader = distortion

        layer = scene.new_shaded_bitmap_layer()
        layer.bitmap = input
        layer.mapping.scale(0.48)
        layer.mapping.rotate_degrees(-2)
        layer.mapping.set_center_position((0.75, 0.75))
        layer.shader = gray_shift

        subscene = beatmup.Scene()
        scene_layer = scene.add_scene(subscene)
        scene_layer.mapping.scale(0.45)
        scene_layer.mapping.rotate_degrees(-3)
        scene_layer.mapping.set_center_position((0.25, 0.25))
        layer = subscene.new_masked_bitmap_layer()
        layer.bitmap = input
        layer.modulation_color = (255, 0, 0, 255)
        layer.mask = beatmup.bitmaptools.chessboard(ctx, 320, 240, 32)

        layer = subscene.new_masked_bitmap_layer()
        layer.bitmap = input
        layer.modulation_color = (255, 255, 0, 128)
        layer.mask = beatmup.bitmaptools.chessboard(ctx, 320, 240, 32)
        beatmup.bitmaptools.invert(layer.mask, layer.mask)

        # setup renderer
        renderer = beatmup.SceneRenderer()
        renderer.scene = scene
        renderer.output = output
        renderer.output_pixels_fetching = True
        renderer.background_image = beatmup.bitmaptools.chessboard(ctx, 16, 16, 8, beatmup.PixelFormat.TRIPLE_BYTE)

        ctx.perform_task(renderer)
        ctx.perform_task(renderer)
        if SAVE_BITMAPS:
            output.save_bmp("test_rendering.bmp")


class ColorMatrixTransformTest(unittest.TestCase):
    def test_color_matrix_transform(self):
        """ Color matrix transformation tests
        """
        ctx = beatmup.Context()
        image = beatmup.bitmaptools.chessboard(ctx, 320, 240, 16, beatmup.PixelFormat.TRIPLE_BYTE)

        matrix = beatmup.filters.ColorMatrix()
        matrix.set_coefficients(0, 0, (1, 0, 0, 0))
        matrix.set_coefficients(1, 0, (0, 0.5, 0, 0))
        matrix.set_coefficients(2, 0, (0, 0, 0.5, 0))
        matrix.input = image
        matrix.output = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.TRIPLE_BYTE, image.get_width(), image.get_height())
        ctx.perform_task(matrix)

        matrix.input = matrix.output
        matrix.output = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.TRIPLE_BYTE, image.get_width(), image.get_height())
        matrix.set_hsv_correction(-90, 1, 1)

        ctx.perform_task(matrix)
        if SAVE_BITMAPS:
            matrix.output.save_bmp("test_colormatrix.bmp")


    def test_sepia(self):
        """ Sepia filter test
        """
        ctx = beatmup.Context()
        image = beatmup.bitmaptools.chessboard(ctx, 320, 240, 16, beatmup.PixelFormat.TRIPLE_BYTE)

        sepia = beatmup.filters.Sepia()
        sepia.input = image
        sepia.output = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.TRIPLE_BYTE, image.get_width(), image.get_height())
        ctx.perform_task(sepia)
        if SAVE_BITMAPS:
            sepia.output.save_bmp("test_sepia.bmp")


class ShaderApplicationTests(unittest.TestCase):
    def test_shader_applicator(self):
        """ ShaderApplicator test
        """
        ctx = beatmup.Context()
        applicator = beatmup.ShaderApplicator()
        applicator.output_bitmap = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.TRIPLE_BYTE, 640, 480)
        applicator.add_sampler(beatmup.bitmaptools.chessboard(ctx, 320, 240, 32, beatmup.PixelFormat.TRIPLE_BYTE))
        self.assertFalse(applicator.remove_sampler("testSampler"))
        applicator.add_sampler(beatmup.bitmaptools.chessboard(ctx, 100, 100, 1, beatmup.PixelFormat.SINGLE_BYTE), "testSampler")
        self.assertTrue(applicator.remove_sampler("testSampler"))
        applicator.add_sampler(beatmup.bitmaptools.chessboard(ctx, 100, 100, 1, beatmup.PixelFormat.SINGLE_BYTE), "testSampler")

        applicator.shader = beatmup.ImageShader(ctx)
        applicator.shader.set_source_code(beatmup.ImageShader.CODE_HEADER + """
            uniform sampler2D testSampler;
            highp vec2 distort(vec2 xy) {
                highp vec2 r = xy - vec2(0.5, 0.5);
                highp float t = length(r);
                return (-0.5 * t * t + 0.9) * r + vec2(0.5, 0.5);
            }
            void main() {
                gl_FragColor = texture2D(image, distort(texCoord)) * texture2D(testSampler, texCoord).r;
            }
            """)

        ctx.perform_task(applicator)

        if SAVE_BITMAPS:
            applicator.output_bitmap.save_bmp("test_shader_applicator.bmp")


class MultitaskTests(unittest.TestCase):
    def test_multitask(self):
        """ Multitask test
        """
        ctx = beatmup.Context()
        applicator = beatmup.ShaderApplicator()
        applicator.output_bitmap = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.TRIPLE_BYTE, 640, 480)
        applicator.add_sampler(beatmup.bitmaptools.chessboard(ctx, 320, 240, 32, beatmup.PixelFormat.TRIPLE_BYTE))

        applicator.shader = beatmup.ImageShader(ctx)
        applicator.shader.set_source_code(beatmup.ImageShader.CODE_HEADER + """
            uniform highp float factor;
            highp vec2 distort(vec2 xy) {
                highp vec2 r = xy - vec2(0.5, 0.5);
                highp float t = length(r);
                return (-factor * t * t + 0.9) * r + vec2(0.5, 0.5);
            }
            void main() {
                gl_FragColor = texture2D(image, distort(texCoord));
            }
            """)
        applicator.shader.set_float('factor', 0.9)

        sepia = beatmup.filters.Sepia()
        sepia.input = applicator.output_bitmap
        sepia.output = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.TRIPLE_BYTE, sepia.input.get_width(), sepia.input.get_height())

        # create a scene
        scene = beatmup.Scene()
        layer = scene.new_bitmap_layer()
        layer.bitmap = sepia.output
        layer.mapping.scale(0.95)
        layer.mapping.rotate_degrees(1)
        layer.mapping.set_center_position((0.5, 0.5))

        # setup renderer
        renderer = beatmup.SceneRenderer()
        renderer.scene = scene
        renderer.output = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.TRIPLE_BYTE, 640, 480)
        renderer.output_pixels_fetching = True
        renderer.background_image = beatmup.bitmaptools.chessboard(ctx, 16, 16, 8, beatmup.PixelFormat.TRIPLE_BYTE)

        # construct multitask and test its API
        multitask = beatmup.Multitask()
        holder = multitask.add_task(renderer)
        self.assertEqual(multitask.get_task_count(), 1)
        holder = multitask.insert_task(applicator, holder)
        self.assertEqual(multitask.get_task_count(), 2)
        self.assertEqual(multitask.remove_task(holder), True)
        multitask.insert_task(sepia, multitask.get_task(0))
        self.assertEqual(multitask.remove_task(holder), False)
        self.assertEqual(multitask.get_task_count(), 2)
        distort_holder = multitask.insert_task(applicator, multitask.get_task(0))
        self.assertEqual(multitask.get_task_index(multitask.get_task(2)), 2)
        multitask.measure()

        # run multitask twice for different parameters: expecting different results
        ctx.perform_task(multitask)
        ref_output = renderer.output
        renderer.output = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.TRIPLE_BYTE, 640, 480)
        applicator.shader.set_float('factor', 0.5)
        multitask.set_repetition_policy(distort_holder, beatmup.Multitask.RepetitionPolicy.REPEAT_UPDATE)
        ctx.perform_task(multitask)
        self.assertLess(beatmup.Metric.psnr(renderer.output, ref_output), 40)

        # run again after setting REPEAT_UPDATE: expecting same result
        ref_output = renderer.output
        renderer.output = beatmup.InternalBitmap(ctx, beatmup.PixelFormat.TRIPLE_BYTE, 640, 480)
        applicator.shader.set_float('factor', 0.9)
        ctx.perform_task(multitask)
        self.assertTrue(beatmup.Metric.psnr(renderer.output, ref_output) > 40)

        # run again after reseting the policy: expecting different results
        multitask.set_repetition_policy(distort_holder, beatmup.Multitask.RepetitionPolicy.REPEAT_UPDATE)
        ctx.perform_task(multitask)
        self.assertTrue(beatmup.Metric.psnr(renderer.output, ref_output) < 40)

        if SAVE_BITMAPS:
            renderer.output.save_bmp("test_multitask.bmp")


class MetricTests(unittest.TestCase):
    def test_metric(self):
        """ Metric test
        """
        ctx = beatmup.Context()
        metric = beatmup.Metric()
        import numpy

        # generate random inputs
        array1 = numpy.random.uniform(-0.5, 0.5, (256, 256, 3)).astype(numpy.float32)
        array2 = numpy.random.uniform(-0.5, 0.5, (256, 256, 3)).astype(numpy.float32)
        diff = (array1 - array2).reshape(-1)
        img1 = beatmup.Bitmap(ctx, array1)
        img2 = beatmup.Bitmap(ctx, array2)
        metric.set_bitmaps(img1, img2)

        # check L1
        metric.set_norm(metric.Norm.L1)
        ctx.perform_task(metric)
        self.assertAlmostEqual(numpy.linalg.norm(diff, 1), metric.get_result(), 1)

        # check L2
        metric.set_norm(metric.Norm.L2)
        ctx.perform_task(metric)
        self.assertAlmostEqual(numpy.linalg.norm(diff, 2), metric.get_result(), 2)

        # check PSNR
        psnr = 10 * numpy.log10(1 / numpy.mean(diff ** 2))
        self.assertAlmostEqual(beatmup.Metric.psnr(img1, img2), psnr, 5)


if __name__ == '__main__':
    unittest.main()
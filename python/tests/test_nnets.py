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
#
# NNets module tests
#
# This script tests NNets API in a unittest fashion and also checks numerical accuracy of the Beatmup backend.
# The latter tests consist of the following identical steps:
#  - building a small keras model,
#  - converting it into a Beatmup model,
#  - running the inference of the two models on the same random input,
#  - comparing the outputs (measuring maximum absolute difference).
# The test is marked as passed if the difference is less than a manually tuned threshold.
#
# In addition, the accuracy tests are all exported into a chunkfile, namely:
#  - the test models in a text format used by Beatmup,
#  - the test models data as floating point tables,
#  - the random test inputs and the reference (TensorFlow) outputs as integer/float tensors,
#  - the error thresholds and short tests descriptions.
# This allows to replay the tests in a constrained environment without recomputing the reference data with Tensorflow
# on the fly.
#  - The same script may be invoked with "--replay" to use the previously generated chunkfile. This is convenient on
#    Raspberry Pi without TensorFlow installed.
#  - The generated chunkfile is used by Tests app (see apps/tests/app.cpp) to replay tests in a Pythonless environment
#    such as Android.
#

import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '1234'    # disabling Tensorflow logging

import beatmup
import beatmup_keras
import numpy
from numpy.random import randint
import struct
import unittest
from packaging import version


# counter of the models written to the test data file
test_export_filename = 'tests.chunks'
model_ctr = 1


def make_random_image(image_size):
    assert len(image_size) == 2
    return numpy.random.randint(0, 255, image_size + (3,)).astype(numpy.uint8)


def make_tensorflow_batch(image):
    assert image.dtype == numpy.uint8
    return tf.expand_dims(tf.cast(image, tf.float32) / 255, axis=0)


def export_test(test_id, title, model_data, test_model, input_image, ref_output, error_threshold):
    """ Writes a test instance data into a chunkfile for a further replay.
    """
    global model_ctr
    if test_export_filename:
        model_data[test_id] = title.strip().encode('ascii')
        model_data[test_id + ':model'] = test_model.serialize().encode('ascii')
        model_data[test_id + ':input'] = input_image
        model_data[test_id + ':input_shape'] = numpy.asarray(input_image.shape, dtype=numpy.int32)
        model_data[test_id + ':gt'] = ref_output
        model_data[test_id + ':threshold'] = struct.pack('f', error_threshold)
        model_data.save(test_export_filename, model_ctr > 1)
        model_ctr += 1


def test_model(error_threshold):
    """ Decorator of functions returning (an input image, a reference keras model) doing the following:
        - compute the reference output for the given input,
        - convert the reference keras model into a Beatmup model,
        - run inference of the converted model on the input image,
        - export the test data,
        - compare the test output with the reference output, raise if too different.
    Args of the decorator itself:
    :error_threshold:
    """
    def wrap(func):
        def wrapped(self, *args, **kwargs):
            input_image, ref_model = func(self, *args, **kwargs)

            # get "test id" to add as prefix to model layers
            global model_ctr
            test_id = 'test' + str(model_ctr)

            # compute reference output
            ref_model.compile()
            ref_output = ref_model.predict(make_tensorflow_batch(input_image))[0]

            # convert model
            ctx = beatmup.Context()
            test_model, model_data = beatmup_keras.export_model(ref_model, ctx, prefix=test_id + '__')

            # init inference task
            inference = beatmup.nnets.InferenceTask(test_model, model_data)

            # connect input
            inference.connect(beatmup.Bitmap(ctx, input_image), test_model.get_first_operation())

            # connect output if not softmax; softmax has no outputs
            head = test_model.get_last_operation()
            softmax = 'softmax' in head.name
            if not softmax:
                test_model.add_output(head)

            # run inference
            ctx.perform_task(inference)

            # get data
            test_output = numpy.asarray(head.get_probabilities()) if softmax else test_model.get_output_data(head)

            # compare
            error = numpy.max(numpy.abs(test_output - ref_output))
            if VERBOSE: print('Error: %0.4f for %d layers mapping %s to %s' % (error, len(ref_model.layers), input_image.shape, test_output.shape))
            del ctx

            # export
            export_test(test_id, func.__doc__, model_data, test_model, input_image, ref_output, error_threshold)

            # assert
            self.assertLess(error, error_threshold)

        return wrapped
    return wrap


def replay_tests():
    """ Reads a file with test data and replays the tests
    """
    # open file
    datafile = beatmup.ChunkFile(test_export_filename)
    datafile.open()

    # prepare things
    ctx = beatmup.Context()

    # loop till tests
    i = 1
    prefix = lambda i: 'test' + str(i)
    while datafile.chunk_exists(prefix(i)):
        # get data
        datafile.open()
        test_title = bytes.decode(datafile[prefix(i)])
        model_code = bytes.decode(datafile[prefix(i) + ':model'])
        input_image_shape = numpy.frombuffer(datafile[prefix(i) + ':input_shape'], dtype=numpy.int32)
        input_image = numpy.frombuffer(datafile[prefix(i) + ':input'], dtype=numpy.uint8).reshape(input_image_shape)
        ref_output = numpy.frombuffer(datafile[prefix(i) + ':gt'], dtype=numpy.float32)
        threshold, = struct.unpack('f', datafile[prefix(i) + ':threshold'])
        datafile.close()
        print('#%02d: %s' % (i, test_title))

        # restore model
        model = beatmup.nnets.DeserializedModel(ctx, model_code)

        # build inference task
        inference = beatmup.nnets.InferenceTask(model, datafile)

        # connect input
        inference.connect(beatmup.Bitmap(ctx, input_image), model.get_first_operation())

        # connect output if not softmax; softmax has no outputs
        head = model.get_last_operation()
        softmax = 'softmax' in head.name
        if not softmax:
            model.add_output(head)

        # run inference
        ctx.perform_task(inference)

        # get data
        test_output = numpy.asarray(head.get_probabilities()) if softmax else model.get_output_data(head)

        # get ground truth data
        ref_output = ref_output.reshape(test_output.shape)

        # check error and print things
        error = numpy.max(numpy.abs(test_output - ref_output))
        print('  Error: %0.4f for mapping %s to %s' % (error, input_image.shape, test_output.shape))
        assert error < threshold

        i += 1


class ChunkCollectionTest(unittest.TestCase):
    def test(self):
        """ Tests basic operations with WritableChunkCollection
        """
        cc = beatmup.WritableChunkCollection()
        x = numpy.random.random((10, 20, 30)).astype(numpy.float32)
        cc["one"] = x
        self.assertTrue(cc.chunk_exists("one"))
        self.assertEqual(cc["one"].shape, x.shape)
        self.assertEqual(cc.chunk_size("one"), x.size * 4)
        self.assertTrue(numpy.all(cc["one"] == x))
        self.assertFalse(cc.chunk_exists("two"))
        self.assertEqual(cc["two"], None)
        cc["two"] = numpy.asarray([1, 2, 3])
        self.assertTrue(numpy.all(cc["two"] == numpy.asarray([1, 2, 3])))
        # string
        cc["bytes"] = "string\nstring".encode("ascii")
        self.assertTrue(bytes.decode(cc["bytes"], "ascii") == "string\nstring")


class Conv2DTests(unittest.TestCase):
    @test_model(0.003)
    def single_conv_test(self, image_size, kernel_size=3, channels=32, stride=1, activation_function=beatmup_keras.brelu1, bias=True):
        """ Single convolution layer test
        """
        # generate input image
        input_image = make_random_image(image_size)

        # set up a test model
        return input_image, tf.keras.models.Sequential([
            tf.keras.layers.Input(input_image.shape),
            tf.keras.layers.Conv2D(channels, kernel_size,
                name='conv',
                strides=stride,
                kernel_initializer='random_normal',
                bias_initializer='random_normal',
                use_bias=bias),
            tf.keras.layers.Activation(activation_function)
        ])


    def test_single_conv(self):
        """ Runs the single convolution layer test on a grid of parameters
        """
        if VERBOSE: print('---- Single Conv2D...')
        activation_functions = [beatmup_keras.brelu1, beatmup_keras.brelu6, beatmup_keras.sigmoid_like]
        for kernel_size in [1, 2, 3, 5]:
            for channels in [4, 32]:
                for stride in [1, 2, 5]:
                    for activation_function in activation_functions:
                        image_size = (randint(kernel_size, 224), randint(kernel_size, 224))
                        self.single_conv_test(image_size=image_size,
                                              kernel_size=kernel_size,
                                              channels=channels,
                                              stride=stride,
                                              activation_function=activation_function)


    @test_model(0.005)
    def double_conv_with_shuffle_test(self, image_size, in_channels, shuffle_step=1):
        """ Two Conv2D with Shuffle test
        """
        # generate input image
        input_image = make_random_image(image_size)

        # set up a test model
        return input_image, tf.keras.models.Sequential([
            tf.keras.layers.Input(input_image.shape),
            tf.keras.layers.Conv2D(in_channels, 3,
                    name='conv1',
                    strides=2,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=True),
            tf.keras.layers.Activation(beatmup_keras.brelu6),
            beatmup_keras.Shuffle(step=shuffle_step),
            tf.keras.layers.Conv2D(16, 1,
                    name='conv2',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    padding='same',
                    use_bias=True),
            tf.keras.layers.Activation(beatmup_keras.brelu6)
        ])


    def test_shuffle(self):
        """ Runs the shuffle test with different shuffling steps
        """
        if VERBOSE: print('---- Two Conv2D with shuffling...')
        for i in [1, 2, 4, 5, 10, 20]:
            self.double_conv_with_shuffle_test((56, 57), 80, i)


    @test_model(0.0085)
    def separable_conv_with_residual_connection_test(self, image_size, channels):
        """ Separable convolution with residual connection test
        """
        # generate input image
        input_image = make_random_image(image_size)

        # set up a test model
        input = tf.keras.layers.Input(input_image.shape)
        x = tf.keras.layers.Conv2D(channels, 3,
                    name='conv',
                    strides=2,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=False)(input)
        x = residual = tf.keras.layers.Activation(beatmup_keras.brelu6)(x)
        x = tf.keras.layers.DepthwiseConv2D(3,
                    name='depthwise_conv',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    padding='same',
                    use_bias=True)(x)
        x = tf.keras.layers.Activation(beatmup_keras.brelu6)(x)
        x = tf.keras.layers.Conv2D(channels, 1,
                    name='pointwise_conv',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    padding='same',
                    use_bias=True)(x)
        x = tf.keras.layers.Add(name="add_residual")([x, residual])
        x = tf.keras.layers.Activation(beatmup_keras.brelu1)(x)
        return input_image, tf.keras.models.Model(inputs=input, outputs=x)


    def test_separable_with_residual_connection(self):
        """ Runs several separable convolution tests with a residual connection
        """
        if VERBOSE: print('---- Separable Conv2D with a residual connection...')
        self.separable_conv_with_residual_connection_test((112, 112), 16)
        self.separable_conv_with_residual_connection_test((56, 56), 32)
        self.separable_conv_with_residual_connection_test((14, 14), 40)
        self.separable_conv_with_residual_connection_test((12, 34), 64)


    @test_model(0.005)
    def group_conv_test(self, image_size, in_channels, out_channels, num_groups):
        """ Group Conv2D test
        """
        # generate input image
        input_image = make_random_image(image_size)

        # set up a test model
        return input_image, tf.keras.models.Sequential([
            tf.keras.layers.Input(input_image.shape),
            tf.keras.layers.Conv2D(in_channels, 3,
                    name='conv1',
                    strides=2,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=True),
            tf.keras.layers.Activation(beatmup_keras.brelu6),
            tf.keras.layers.Conv2D(out_channels, 3,
                    name='conv2',
                    groups=num_groups,
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    padding='same',
                    use_bias=True),
            tf.keras.layers.Activation(beatmup_keras.brelu6)
        ])


    def test_group_conv(self):
        """ Runs few group convolution tests
        """
        if version.parse(tf.__version__) < version.parse('2.3.0'):
            self.skipTest("Group convolutions are unsupported by TensorFlow prior to v2.3.0")
        if VERBOSE: print('---- Group convolution...')
        self.group_conv_test((56, 57), 16, 32, 2)
        self.group_conv_test((56, 57), 16, 32, 4)
        self.group_conv_test((56, 57), 64, 64, 8)
        self.group_conv_test((56, 57), 64, 64, 16)


class PoolingTests(unittest.TestCase):
    @test_model(0.0048)
    def pooling_test(self, pool_op, image_size, size, stride, padding='VALID'):
        """ Pooling test
        """
        # generate input image
        input_image = make_random_image(image_size)

        # set up a test model
        return input_image, tf.keras.models.Sequential([
            tf.keras.layers.Input(input_image.shape),
            tf.keras.layers.Conv2D(32, 3,
                    name='conv',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=True),
            tf.keras.layers.Activation(beatmup_keras.brelu6),
            pool_op(size, name='pool', strides=stride, padding=padding)
        ])

    def test_maxpool(self):
        """ Runs the max pooling test on a grid of parameters
        """
        if VERBOSE: print('---- Max pooling...')
        for size in [2, 3, 5]:
            for stride in [1, 2, 5]:
                for padding in ['VALID', 'SAME']:
                    image_size = (randint(size+2, 224), randint(size+2, 224))
                    self.pooling_test(tf.keras.layers.MaxPooling2D, image_size, size, stride, padding)

    def test_avgpool(self):
        """ Runs the average pooling test on a grid of parameters
        """
        if VERBOSE: print('---- Average pooling...')
        for size in [2, 3, 4, 5]:
            for stride in [1, 2, 5]:
                image_size = (randint(size+2, 224), randint(size+2, 224))
                self.pooling_test(tf.keras.layers.AveragePooling2D, image_size, size, stride)


class GlobalPoolingTests(unittest.TestCase):
    @test_model(0.0044)
    def global_pooling_test(self, pool_op, image_size):
        """ Global pooling test
        """
        # generate input image
        input_image = make_random_image(image_size)

        # set up a test model
        return input_image, tf.keras.models.Sequential([
            tf.keras.layers.Input(input_image.shape),
            tf.keras.layers.Conv2D(32, 2,
                    name='conv',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=True),
            tf.keras.layers.Activation(beatmup_keras.brelu6),
            pool_op(name='pool'),
        ])

    def test_global_maxpool(self):
        """ Runs the global max pooling test for different input sizes
        """
        if VERBOSE: print('---- Global max pooling...')
        for size in range(3, 7):
            self.global_pooling_test(tf.keras.layers.GlobalMaxPooling2D, (size, size))

    def test_global_avgpool(self):
        """ Runs the global average pooling test for different input sizes
        """
        if VERBOSE: print('---- Global average pooling...')
        for size in range(3, 7):
            self.global_pooling_test(tf.keras.layers.AveragePooling2D, (size, size))


class DenseTests(unittest.TestCase):
    @test_model(0.0078126)
    def single_dense_layer_test(self, image_size, in_channels, out_channels):
        """ Dense layer test
        """
        # generate input image
        input_image = make_random_image(image_size)

        # set up a test model
        return input_image, tf.keras.models.Sequential([
            tf.keras.layers.Input(input_image.shape),
            tf.keras.layers.Conv2D(in_channels, 3,
                name='conv',
                strides=1,
                kernel_initializer='random_normal',
                bias_initializer='random_normal',
                use_bias=False),
            tf.keras.layers.Activation(beatmup_keras.brelu6),
            tf.keras.layers.GlobalMaxPooling2D(),
            tf.keras.layers.Dense(out_channels,
                name='dense',
                kernel_initializer='random_normal',
                bias_initializer='random_normal',
                use_bias=True)
        ])

        # test the model

    def test_single_dense(self):
        """ Runs the dense layer test on a grid of parameters
        """
        if VERBOSE: print('---- Dense layer...')
        for in_channels in [8, 16, 32, 64]:
            for out_channels in [4, 8, 16, 64]:
                for size in range(4, 8):
                    self.single_dense_layer_test((size, size), in_channels, out_channels)

    @test_model(0.0086)
    def two_dense_layers_test(self, image_size, in_channels, mid_channels, out_channels):
        """ Two dense layers test
        """
        # generate input image
        input_image = make_random_image(image_size)

        # set up a test model
        return input_image, tf.keras.models.Sequential([
            tf.keras.layers.Input(input_image.shape),
            tf.keras.layers.Conv2D(in_channels, 3,
                name='conv',
                strides=1,
                kernel_initializer='random_normal',
                bias_initializer='random_normal',
                use_bias=False),
            tf.keras.layers.Activation(beatmup_keras.brelu6),
            tf.keras.layers.GlobalMaxPooling2D(),
            tf.keras.layers.Dense(mid_channels,
                name='dense1',
                kernel_initializer='random_normal',
                bias_initializer='random_normal',
                use_bias=False),
            tf.keras.layers.Dense(out_channels,
                name='dense2',
                kernel_initializer='random_normal',
                bias_initializer='random_normal',
                use_bias=True)
        ])

    def test_two_dense_layers(self):
        """ Runs few 2 dense layers tests
        """
        if VERBOSE: print('---- Two dense layers...')
        self.two_dense_layers_test((7, 7), 32, 32, 32)
        self.two_dense_layers_test((7, 7), 16, 32, 64)
        self.two_dense_layers_test((7, 7), 64, 96, 128)
        self.two_dense_layers_test((7, 7), 48, 24, 48)


class BatchNormalizationTests(unittest.TestCase):
    @test_model(0.003)
    def batch_norm_test(self, image_size):
        """ Batch normalization test
        """
        # generate input image
        input_image = make_random_image(image_size)

        # set up a test model
        return input_image, tf.keras.models.Sequential([
            tf.keras.layers.Input(input_image.shape),
            tf.keras.layers.Conv2D(16, 1,
                name='conv',
                strides=1,
                kernel_initializer='random_normal',
                bias_initializer='random_normal',
                use_bias=False),
            tf.keras.layers.BatchNormalization(
                beta_initializer='random_normal',
                gamma_initializer='random_normal',
                moving_mean_initializer='random_normal',
                moving_variance_initializer=tf.keras.initializers.RandomUniform(0, 100)),
            tf.keras.layers.Activation(beatmup_keras.brelu6),
        ])


    def test_batch_normalization(self):
        """ Runs few batch normalization tests
        """
        if VERBOSE: print('---- Batch normalization...')
        self.batch_norm_test((32, 32))
        self.batch_norm_test((123, 45))


class SoftmaxTests(unittest.TestCase):
    @test_model(0.001)
    def softmax_test(self, image_size, channels):
        """ Softmax test
        """
        # generate input image
        input_image = make_random_image(image_size)

        # set up a test model
        return input_image, tf.keras.models.Sequential([
            tf.keras.layers.Input(input_image.shape),
            tf.keras.layers.Conv2D(channels, 1,
                name='conv',
                strides=1,
                kernel_initializer='random_normal',
                bias_initializer='random_normal',
                use_bias=False),
            tf.keras.layers.Activation(beatmup_keras.brelu6),
            beatmup_keras.Shuffle(),
            tf.keras.layers.GlobalMaxPooling2D(),
            tf.keras.layers.Softmax()
        ])

    def test_softmax_layer(self):
        if VERBOSE: print('---- Softmax...')
        self.softmax_test((5, 5), 32)
        self.softmax_test((4, 4), 64)
        self.softmax_test((3, 3), 192)


class SerializationTest(unittest.TestCase):
    def test_serialization(self):
        """ Tests model serialization and reconstruction
        """
        if VERBOSE: print('---- Serialization...')

        # generate input image
        input_image = make_random_image((32, 32))

        # set up a test model
        input = tf.keras.layers.Input(input_image.shape)
        x = tf.keras.layers.Conv2D(32, 3,
                    name='conv_1',
                    strides=2,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=False)(input)
        x = residual = tf.keras.layers.Activation(beatmup_keras.brelu6)(x)
        x = tf.keras.layers.DepthwiseConv2D(3,
                    name='depthwise_conv_2',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    padding='same',
                    use_bias=True)(x)
        x = tf.keras.layers.Activation(beatmup_keras.brelu6)(x)
        x = tf.keras.layers.Conv2D(32, 1,
                    name='pointwise_conv_2',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=True)(x)
        x = tf.keras.layers.Add(name="add_residual_1")([x, residual])
        x = tf.keras.layers.Activation(beatmup_keras.brelu1)(x)

        x = tf.keras.layers.MaxPooling2D(2)(x)
        x = tf.keras.layers.Conv2D(64, 1,
                    name='pointwise_conv',
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=True)(x)
        x = tf.keras.layers.ReLU(max_value=2.0)(x)
        x = residual = beatmup_keras.Shuffle(2)(x)

        x = tf.keras.layers.DepthwiseConv2D(3,
                    name='depthwise_conv_3',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    padding='same',
                    use_bias=True)(x)
        x = tf.keras.layers.Activation(beatmup_keras.brelu6)(x)
        x = tf.keras.layers.Conv2D(64, 1,
                    name='pointwise_conv_3',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=True)(x)
        x = tf.keras.layers.Add(name="add_residual_2")([x, residual])
        x = tf.keras.layers.ReLU(max_value=2.0)(x)

        x = tf.keras.layers.GlobalAveragePooling2D()(x)
        x = tf.keras.layers.Dense(40)(x)
        x = tf.keras.layers.Softmax()(x)

        # make a model
        ref_model = tf.keras.models.Model(inputs=input, outputs=x)
        ref_model.compile()
        ref_output = ref_model.predict(make_tensorflow_batch(input_image))[0]

        # convert model
        ctx = beatmup.Context()
        model, model_data = beatmup_keras.export_model(ref_model, ctx)

        # run inference
        inference = beatmup.nnets.InferenceTask(model, model_data)
        inference.connect(beatmup.Bitmap(ctx, input_image), model.get_first_operation())
        ctx.perform_task(inference)
        output = model.get_last_operation().get_probabilities()

        # print stuff
        error = numpy.max(numpy.abs(output- ref_output))
        if VERBOSE: print("Error: %0.4f for %d layers mapping %s to %s" % (error, len(ref_model.layers), input_image.shape, len(output)))

        # serialize
        serial = model.serialize()

        # reconstruct
        reconstructed_model = beatmup.nnets.DeserializedModel(ctx, serial)

        # run inference of the reconstructed model
        inference_rec = beatmup.nnets.InferenceTask(reconstructed_model, model_data)
        inference_rec.connect(beatmup.Bitmap(ctx, input_image), reconstructed_model.get_first_operation())
        ctx.perform_task(inference_rec)
        output_rec = model.get_last_operation().get_probabilities()

        # compare
        self.assertEqual(output, output_rec)


class ImageSamplerTest(unittest.TestCase):
    def test_image_sampler(self):
        """ ImageSampler test
        """
        if VERBOSE: print('---- ImageSampler test...')

        # generate input image
        input_image = make_random_image((48, 32))
        center_crop = input_image[8:-8,:,:]

        # set up a test model
        ref_model = tf.keras.models.Sequential([
            tf.keras.layers.Input(input_image.shape),
            tf.keras.layers.Conv2D(8, 1,
                name='conv',
                strides=1,
                kernel_initializer='random_normal',
                bias_initializer='random_normal',
                use_bias=False),
            tf.keras.layers.Activation(beatmup_keras.brelu1)
        ])

        # get "test id" to add as prefix to model layers
        global model_ctr
        test_id = 'test' + str(model_ctr)

        # convert model
        ctx = beatmup.Context()
        model, model_data = beatmup_keras.export_model(ref_model, ctx, prefix=test_id + '__')

        # run inference on a cropped input
        inference = beatmup.nnets.InferenceTask(model, model_data)
        inference.connect(beatmup.Bitmap(ctx, center_crop), model.get_first_operation())
        model.add_output(model.get_last_operation())
        ctx.perform_task(inference)
        ref_output = model.get_output_data(model.get_last_operation())

        # add a preprocessing layer
        model, model_data = beatmup_keras.export_model(ref_model, ctx, prefix=test_id + '__')
        image_sampler = beatmup.nnets.ImageSampler(test_id + "__preprocessing", center_crop.shape[:2])
        first_op = model.get_first_operation()
        model.add_operation(first_op.name, image_sampler)
        model.add_connection(image_sampler.name, first_op.name)

        # run on full input
        inference = beatmup.nnets.InferenceTask(model, model_data)
        inference.connect(beatmup.Bitmap(ctx, input_image), model.get_first_operation())
        model.add_output(model.get_last_operation())
        ctx.perform_task(inference)
        test_output = model.get_output_data(model.get_last_operation())

        self.assertTrue(numpy.all(ref_output == test_output))

        # export the test
        export_test(test_id, self.test_image_sampler.__doc__, model_data, model, input_image, ref_output, 0.004)


    def test_rotation(self):
        """ ImageSampler rotation test
        """
        if VERBOSE: print('---- ImageSampler rotation test...')

        # generate input image
        input_image = make_random_image((48, 32))
        center_crop = input_image[8:-8,:,:]

        # set up a test model
        ref_model = tf.keras.models.Sequential([
            tf.keras.layers.Input(input_image.shape),
            tf.keras.layers.Conv2D(4, 1,
                name='conv',
                strides=1,
                kernel_initializer='random_normal',
                bias_initializer='random_normal',
                use_bias=False),
            tf.keras.layers.Activation(beatmup_keras.brelu1)
        ])

        # convert model
        ctx = beatmup.Context()
        model, model_data = beatmup_keras.export_model(ref_model, ctx)

        # add a preprocessing layer
        image_sampler = beatmup.nnets.ImageSampler('sampler', center_crop.shape[:2])
        first_op = model.get_first_operation()
        model.add_operation(first_op.name, image_sampler)
        model.add_connection(image_sampler.name, first_op.name)

        # run inference on a cropped input
        inference = beatmup.nnets.InferenceTask(model, model_data)
        inference.connect(beatmup.Bitmap(ctx, center_crop), model.get_first_operation())
        model.add_output(model.get_last_operation())
        ctx.perform_task(inference)
        ref_output = model.get_output_data(model.get_last_operation())

        # rotate and test
        for i in range(4):
            image_sampler.rotation = i
            ctx.perform_task(inference)
            test_output = model.get_output_data(model.get_last_operation())
            self.assertTrue(numpy.all(ref_output == numpy.rot90(test_output, i)))


class ReLUTests(unittest.TestCase):
    @test_model(0.0028)
    def basic_relu_test(self, max_value):
        """ Basic ReLU test
        """
        # generate input image
        input_image = make_random_image((3, 3))

        # set up a test model
        model = tf.keras.models.Sequential([
            tf.keras.layers.Conv2D(8, 3,
                        name='conv',
                        kernel_initializer='random_normal',
                        bias_initializer='random_normal',
                        use_bias=False),
            tf.keras.layers.BatchNormalization(
                        beta_initializer='random_normal',
                        gamma_initializer='random_normal',
                        moving_mean_initializer='random_normal',
                        moving_variance_initializer=tf.keras.initializers.RandomUniform(0, 100)),
            tf.keras.layers.ReLU(max_value=max_value),
            tf.keras.layers.Flatten(),
            tf.keras.layers.Dense(8,
                        name='dense',
                        kernel_initializer='random_normal',
                        bias_initializer='random_normal',
                        use_bias=True)
        ])
        return input_image, model

    def test_two_conv2d_layers(self):
        """ Runs few basic ReLU tests
        """
        if VERBOSE: print('---- Basic ReLU test...')
        self.basic_relu_test(0.5)
        self.basic_relu_test(2.0)
        self.basic_relu_test(10.0)


    @test_model(0.009)
    def residual_connection_test(self, image_size, max_value):
        """ Residual connection test
        """
        # generate input image
        input_image = make_random_image(image_size)

        # set up a test model
        input = tf.keras.layers.Input(input_image.shape)
        x = tf.keras.layers.Conv2D(16, 3,
                    name='conv',
                    strides=2,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=False)(input)
        x = residual = tf.keras.layers.ReLU(max_value=max_value)(x)
        x = tf.keras.layers.DepthwiseConv2D(3,
                    name='depthwise_conv',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    padding='same',
                    use_bias=True)(x)
        x = tf.keras.layers.ReLU(max_value=max_value)(x)
        x = tf.keras.layers.Conv2D(16, 1,
                    name='pointwise_conv',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=True)(x)
        x = tf.keras.layers.Add(name="add_residual")([x, residual])
        x = tf.keras.layers.ReLU(max_value=max_value)(x)
        x = tf.keras.layers.Conv2D(32, 1,
                    name='pointwise_conv_2',
                    strides=1,
                    kernel_initializer='random_normal',
                    bias_initializer='random_normal',
                    use_bias=True)(x)
        x = tf.keras.layers.ReLU(max_value=1.0)(x)
        return input_image, tf.keras.models.Model(inputs=input, outputs=x)


    def test_residual_connection(self):
        """ Runs residual connection tests
        """
        if VERBOSE: print('---- Residual connection tests...')
        self.residual_connection_test((112, 112), 0.5)
        self.residual_connection_test((56, 56), 2.0)
        self.residual_connection_test((14, 14), 6.0)


class ModelStatsTest(unittest.TestCase):
    def test_multiply_adds_and_texel_fetches(self):
        """ Tests multiply-adds and texel fetches counting
        """
        # generate input image
        input_image = make_random_image((32, 32))

        # set up a test model
        model = tf.keras.models.Sequential([
            tf.keras.layers.Input((32, 32, 3)),
            tf.keras.layers.Conv2D(16, kernel_size=3, use_bias=False),
            tf.keras.layers.Activation(beatmup_keras.brelu6),
            tf.keras.layers.Conv2D(32, kernel_size=3, groups=4, use_bias=False),
            tf.keras.layers.Activation(beatmup_keras.brelu6),
            tf.keras.layers.MaxPooling2D(3, strides=1)
        ])

        # prepare model
        ctx = beatmup.Context()
        test_model, test_data = beatmup_keras.export_model(model, ctx)
        inference = beatmup.nnets.InferenceTask(test_model, test_data)
        inference.connect(beatmup.Bitmap(ctx, input_image), test_model.get_first_operation())
        test_model.add_output(test_model.get_last_operation())
        ctx.perform_task(inference)

        # check
        self.assertEqual(test_model.count_multiply_adds(), 30*30*16*3*3*3 + 28*28*32*3*3*4 + 0)
        self.assertEqual(test_model.count_texel_fetches(), 30*30*16*3*3//4 + 28*28*32*3*3//4 + 26*26*32*3*3//4)


if __name__ == '__main__':
    import sys

    # replaying: if tests were run before and their data (models, inputs and ground truth outputs) is stored to a file,
    # we can rerun the same tests without bothering TensorFlow:
    if '--replay' in sys.argv:
        replay_tests()
        exit(0)

    # configure Tensorflow
    import tensorflow as tf
    gpus = tf.config.experimental.list_physical_devices('GPU')
    for gpu in gpus:
        tf.config.experimental.set_memory_growth(gpu, True)

    tf.get_logger().setLevel('ERROR')
    VERBOSE = False

    # tweak verbosity
    if not ('-q' in sys.argv or '--quiet' in sys.argv):
        unittest_verbosity=0
        VERBOSE = True
    else:
        unittest_verbosity=1

    unittest.main(verbosity=unittest_verbosity)

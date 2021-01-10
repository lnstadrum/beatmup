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
"""
beatmup_keras module
--------------------

Contains activation functions and Shuffle operation implementation supported in Beatmup neural network inference backend.
Implements keras models conversion utility into Beatmup models.
"""

import struct
import tensorflow as tf


def brelu1(x):
    """ Activation function: ReLU clipped into [0, 1] range.
    Corresponds to :attr:`DEFAULT <beatmup.nnets.ActivationFunction>` activation function.
    """
    return tf.keras.backend.clip(x, 0, 1)


def brelu6(x):
    """ Activation function: 1/6 * ReLU clipped into [0, 1] range.
    Corresponds to :attr:`BRELU6 <beatmup.nnets.ActivationFunction>` activation function.
    This is the activation function used in MobileNet v1 and v2 architectures with the output stretched to 0..1 range.
    This stretching is added to cope with the backend constraints, namely to be able to store the activation values
    into integer-valued textures on low-end devicees.
    """
    return tf.keras.backend.clip(x * 0.167, 0, 1)


class Shuffle(tf.keras.layers.Layer):
    """ Shuffling layer implementation.

    This layer changes the order of feature maps in the tensor in a special way, which allows for an efficient
    inference implementation in the Beatmup engine: no memory copy is done, only texture sampling order changes,
    which has a minimum-to-no impact on the inference speed.
    The shuffle is done by contiguous blocks of 4 feature maps. For shuffling step equal to `n`, the input channels
    are put in the following order on output:

        0, 1, 2, 3, 4n, 4n+1, 4n+2, 4n+3, 8n, 8n+1, 8n+2, 8n+3, ..., 4, 5, 6, 7, 4n+4, 4n+5, 4n+6, 4n+7, 8n+4, ...

    For shuffling step equal to 1 the operation is identity.
    In order to have all the input channels present, the shuffling step times 4 must divide the number of input
    channels.
    """
    def __init__(self, step=8, **kwargs):
        self.step = max(int(step), 1)
        super().__init__(**kwargs)

    def call(self, tensor):
        d = tensor.shape[3] // 4
        tex_ids = [(_ * self.step) % d + (_ * self.step) // d for _ in range(0, d)]
        p = []
        for i in tex_ids:
            p += range(4 * i, 4 * i + 4)
        return tf.gather(tensor, p, axis=3, name=self.name+'_shuffle')

    def get_config(self):
        config = super().get_config()
        config['step'] = self.step
        return config


# adding custom objects to keras to enable model loading
tf.keras.utils.get_custom_objects().update({
    'brelu1': brelu1,
    'brelu6': brelu6,
    'Shuffle': Shuffle
})


class CannotExport(Exception):
    """ Exception thrown when a given model cannot be exported in a form of a Beatmup model
    """
    def check(condition, layer, message):
        if not condition:
            raise CannotExport('"%s": %s' % (layer.name, message))


def export_model(model, context, model_data=None, prefix=""):
    """ Converts a keras model to a Beatmup model.
    Scans a given keras model trying to convert its layers into Beatmup operations. The converted operations are connected into a Beatmup model.

    :param model:         The input keras model
    :param context:       A Beatmup context
    :param model_data:    A :class:`beatmup.WritableChunkCollection` that will store the model data. If None, a new collection is created.
    :param prefix:        A string prefix to add to the operation names.

    Returns the converted model (instance of :class:`beatmup.nnets.Model`) and its corresponding data (``model_data`` extended with the operations data, if provided, else a new collection).
    Raises exceptions if cannot export.
    """
    import beatmup
    import beatmup.nnets as bnn
    import tensorflow.keras.layers as layers
    import numpy

    # a state machine follows. These are state variables:
    conv2d_layer = None     # if set, a Conv2D layer is about to be exported

    # additional state variables
    conv2d_activation_func = None
    conv2d_skip_connection = None
    conv2d_batch_normalization = False

    # various lists and mappings
    ops = []
    connections = []
    nodes = {}                  # represents outputs of exported ops; maps a keras op to (exported keras op, output index, shuffling step)
    processed_layers = set()    # used to check if all the input model layers have been processed

    # init model data if not provided
    if model_data is None:
        model_data = beatmup.WritableChunkCollection()

    def get_input_node(layer, index=0):
        """ Retrieves the node corresponding to a specific input of a given layer
        """
        assert layer._inbound_nodes and len(layer._inbound_nodes) == 1
        source_protos = layer._inbound_nodes[index].inbound_layers
        if isinstance(source_protos, tf.keras.layers.Layer):
            assert index == 0
            source_proto = source_protos
        else:
            source_proto = source_protos[index]
            assert isinstance(source_proto, tf.keras.layers.Layer)
        CannotExport.check(source_proto in nodes, layer, 'layer input %s is not exported' % (source_proto.name))
        return nodes[source_proto]

    # iterate over the layers chain
    def process_layer(layer):
        processed_layers.add(layer)
        # remove quantize wrapping, if any
        if type(layer).__name__ == 'QuantizeAnnotate' or type(layer).__name__ == 'QuantizeWrapper':
            layer = layer.layer
        else:
            layer = layer

        # if state is "exporting Conv2D"
        nonlocal conv2d_layer, conv2d_activation_func, conv2d_skip_connection, conv2d_batch_normalization
        if conv2d_layer:
            # activation
            if isinstance(layer, layers.Activation):
                CannotExport.check(conv2d_activation_func is None, layer, 'activation function is redefined for ' + conv2d_layer.name)

                # setup activation layer
                if layer.activation == brelu1:
                    conv2d_activation_func = bnn.ActivationFunction.DEFAULT
                elif layer.activation == brelu6:
                    conv2d_activation_func = bnn.ActivationFunction.BRELU6
                else:
                    CannotExport.check(False, layer, 'unsupported activation function')

                # Conv2D unit is complete, write out
                ops.append(bnn.Conv2D(
                    prefix + conv2d_layer.name,
                    conv2d_layer.kernel_size[0],
                    conv2d_layer.input_shape[3],
                    conv2d_layer.output_shape[3],
                    conv2d_layer.strides[0],
                    bnn.Padding.VALID if conv2d_layer.padding.lower() == 'valid' else bnn.Padding.SAME,
                    conv2d_layer.use_bias or conv2d_batch_normalization,
                    (conv2d_layer.input_shape[3] if isinstance(conv2d_layer, layers.DepthwiseConv2D) else
                     conv2d_layer.groups         if hasattr(conv2d_layer, 'groups') else 1),
                    conv2d_activation_func))
                nodes[layer] = (conv2d_layer, 0, 0)

                # add connection
                if len(ops) > 1:
                    src, out_idx, shuffle = get_input_node(conv2d_layer)
                    connections.append({
                        'source_op': prefix + src.name,
                        'dest_op': prefix + conv2d_layer.name,
                        'output': out_idx,
                        'input': 0,
                        'shuffle': shuffle
                    })

                # quit "exporting Conv2D" state
                conv2d_layer = None

            # a residual connection
            elif isinstance(layer, layers.Add):
                assert len(layer._inbound_nodes) == 1
                terms = layer._inbound_nodes[0].inbound_layers
                CannotExport.check(len(terms) == 2, layer, "Add layers having two terms are supported only")
                # find a term which is already exported
                for t in terms:
                    if t in nodes:
                        # connect it
                        CannotExport.check(not conv2d_skip_connection, t, 'skip connection is already used after ' + conv2d_layer.name)
                        conv2d_skip_connection = True
                        src, out_idx, shuffle = nodes[t]
                        connections.append({
                            'source_op': prefix + src.name,
                            'dest_op': prefix + conv2d_layer.name,
                            'output': out_idx,
                            'input': 1,
                            'shuffle': shuffle
                        })
                # check a term is found
                if not conv2d_skip_connection:
                    CannotExport.check(layer in nodes, layer, 'layer is not exported to provide a connection to ' + conv2d_layer.name)

            # batch norm
            elif isinstance(layer, layers.BatchNormalization):
                # make sure there is only one batch norm layer
                CannotExport.check(not conv2d_batch_normalization, layer,
                                'another batch normalization is present after ' + conv2d_layer.name)
                # ensure there is no bias in conv2d
                CannotExport.check(not conv2d_layer.use_bias, layer,
                                'bias defined when exporing batch normalization after ' + conv2d_layer.name)

                # get all the tensors
                kernel = model_data[prefix + conv2d_layer.name + bnn.Conv2D.filters_chunk_suffix]
                beta  = layer.beta.numpy().astype(numpy.float32)
                gamma = layer.gamma.numpy().astype(numpy.float32)
                mean = layer.moving_mean.numpy().astype(numpy.float32)
                var  = layer.moving_variance.numpy().astype(numpy.float32)

                # compute scale and bias
                scale = gamma / numpy.sqrt(var + layer.epsilon)
                bias = -mean * scale + beta

                # apply scaling to filter coefficients
                if isinstance(conv2d_layer, layers.DepthwiseConv2D):
                    assert kernel.shape[2] == len(scale)
                    for i in range(kernel.shape[2]):
                        kernel[:,:,i,:] = kernel[:,:,i,:] * scale[i]
                else:
                    assert kernel.shape[3] == len(scale)
                    for i in range(kernel.shape[3]):
                        kernel[:,:,:,i] = kernel[:,:,:,i] * scale[i]

                # write out
                model_data[prefix + conv2d_layer.name + bnn.Conv2D.filters_chunk_suffix] = kernel
                model_data[prefix + conv2d_layer.name + bnn.Conv2D.bias_chunk_suffix] = bias
                conv2d_batch_normalization = True

            else:
                CannotExport.check(False, layer, 'cannot export this layer within Conv2D block %s' % (conv2d_layer.name))

        # process the layer depending on its class
        elif isinstance(layer, layers.Activation):
            CannotExport.check(False, layer, 'activation functions are only supported after Conv2D')

        elif isinstance(layer, layers.BatchNormalization):
            CannotExport.check(False, layer, 'batch normalization layers are only supported after Conv2D')

        elif isinstance(layer, layers.Conv2D):
            CannotExport.check(layer.kernel_size[0] == layer.kernel_size[1], layer, 'squared Conv2D kernels are supported only')
            CannotExport.check(layer.strides[0] == layer.strides[1], layer, 'equal strides in Conv2D are supported only')

            # write out layer data
            model_data[prefix + layer.name + bnn.Conv2D.filters_chunk_suffix] = layer.weights[0].numpy().astype(numpy.float32)
            if layer.use_bias:
                model_data[prefix + layer.name + bnn.Conv2D.bias_chunk_suffix] = layer.bias.numpy().astype(numpy.float32)

            # enabling "exporing Conv2D" state
            conv2d_layer = layer
            conv2d_activation_func = None
            conv2d_skip_connection = False
            conv2d_batch_normalization = False

        elif isinstance(layer, layers.Dense):
            # add op
            ops.append(bnn.Dense(context, prefix + layer.name, layer.units, layer.use_bias))
            nodes[layer] = (layer, 0, 0)
            # add connection
            src, out_idx, shuffle = get_input_node(layer)
            connections.append({
                'source_op': prefix + src.name,
                'dest_op': prefix + layer.name,
                'output': out_idx,
                'input': 0,
                'shuffle': shuffle
            })
            # export chunks
            model_data[prefix + layer.name + bnn.Dense.matrix_chunk_suffix] = layer.weights[0].numpy().transpose().astype(numpy.float32)
            if layer.use_bias:
                model_data[prefix + layer.name + bnn.Dense.bias_chunk_suffix] = layer.bias.numpy().astype(numpy.float32)

        elif isinstance(layer, layers.Flatten):
            # pass-thru
            nodes[layer] = get_input_node(layer)

        elif isinstance(layer, layers.InputLayer):
            pass

        elif isinstance(layer, layers.AveragePooling2D) or isinstance(layer, layers.MaxPooling2D):
            CannotExport.check(layer.pool_size[0] == layer.pool_size[1], layer, 'squared 2D pooling is supported only')
            CannotExport.check(layer.strides[0] == layer.strides[1], layer, 'equal strides in 2D pooling are supported only')

            # add op
            ops.append(bnn.Pooling2D(
                prefix + layer.name,
                bnn.Pooling2D.Operator.MAX if isinstance(layer, layers.MaxPooling2D) else bnn.Pooling2D.Operator.AVERAGE,
                layer.pool_size[0],
                layer.strides[0],
                bnn.Padding.VALID if layer.padding.lower() == 'valid' else bnn.Padding.SAME))
            nodes[layer] = (layer, 0, 0)

            # add connection
            src, out_idx, shuffle = get_input_node(layer)
            connections.append({
                'source_op': prefix + src.name,
                'dest_op': prefix + layer.name,
                'output': out_idx,
                'input': 0,
                'shuffle': shuffle
            })

        elif isinstance(layer, layers.GlobalAveragePooling2D) or isinstance(layer, layers.GlobalMaxPooling2D):
            # add op
            ops.append(bnn.Pooling2D(
                prefix + layer.name,
                bnn.Pooling2D.Operator.MAX if isinstance(layer, layers.GlobalMaxPooling2D) else bnn.Pooling2D.Operator.AVERAGE,
                layer.input_shape[1],
                1, bnn.Padding.VALID))
            nodes[layer] = (layer, 0, 0)

            # add connection
            src, out_idx, shuffle = get_input_node(layer)
            connections.append({
                'source_op': prefix + src.name,
                'dest_op': prefix + layer.name,
                'output': out_idx,
                'input': 0,
                'shuffle': shuffle
            })

        elif isinstance(layer, layers.Softmax):
            # add op
            ops.append(bnn.Softmax(prefix + layer.name))
            nodes[layer] = (layer, 0, 0)

            # add connection
            src, out_idx, shuffle = get_input_node(layer)
            connections.append({
                'source_op': prefix + src.name,
                'dest_op': prefix + layer.name,
                'output': out_idx,
                'input': 0,
                'shuffle': shuffle
            })

        elif isinstance(layer, Shuffle):
            # in Beatmup, Shuffle is not an operation but a connection modifier
            # resolve the exported operation corresponding to the input layer
            op, out_idx, shuffle = get_input_node(layer)
            # make sure no shuffling is done
            CannotExport.check(shuffle == 0, layer, 'cannot have multiple Shuffle layers connected')
            # add new redirection to the original op, but with shuffling
            nodes[layer] = (op, out_idx, layer.step)

        else:
            raise CannotExport('cannot export layer %s, unsupported type %s' % (layer.name, type(layer).__name__))

        # go to next layer
        for node in layer._outbound_nodes:
            if not node.outbound_layer in processed_layers:
                process_layer(node.outbound_layer)
    # /process_layer

    # iterate model inputs
    for input in model.inputs:
        # find layer connected to this input
        for layer in model.layers:
            inputs = layer.inbound_nodes[0].input_tensors
            if isinstance(inputs, tf.Tensor): inputs = [inputs]
            if model.inputs[0] in inputs:
                process_layer(layer)
                break

    # check if no leftovers
    CannotExport.check(not conv2d_layer, conv2d_layer, "activation function is missing")
    CannotExport.check(not isinstance(layer, Shuffle), layer, "a layer after Shuffle is required")
    for layer in model.layers:
        CannotExport.check(layer in processed_layers, layer, "the layer was not discovered during model scanning")

    # build model
    exported_model = bnn.Model(context)
    for op in ops:
        exported_model.append(op)
    for conn in connections:
        exported_model.add_connection(**conn)

    return exported_model, model_data

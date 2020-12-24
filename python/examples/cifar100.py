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
# This script is an example of how to use Beatmup to run inference of a simple model trained with TensorFlow/Keras.
# It contains the following main stages:
#  - train a Beatmup-compliant model on CIFAR100 dataset using keras,
#  - convert the trained keras model into a Beatmup model,
#  - run the inference of the Beatmup model on CIFAR100 test set and meter the top-1 accuracy.
#
# The used model provides very a modest performance (~40% top-1 test accuracy for ~50K parameters and few minutes of training on a desktop GPU),
# but most importantly shows the same accuracy up to an epsilon when inferred with Beatmup.
#


import beatmup
import beatmup_keras
import numpy
import tensorflow as tf


# Some initial GPU preparation for Tensorflow
gpus = tf.config.experimental.list_physical_devices('GPU')
for gpu in gpus:
  tf.config.experimental.set_memory_growth(gpu, True)


# Get CIFAR100 dataset
(train_images, train_labels), (test_images, test_labels) = tf.keras.datasets.cifar100.load_data()

# Normalize data to 0..1 range
# Tensorflow does not need it, but Beatmup does: the image data is expected to be in 0..1 range.
train_images = train_images.astype(numpy.float32) / 255.0
test_images = test_images.astype(numpy.float32) / 255.0


# Build a simple model: few grouped convolutions with batch normalization and a Beatmup-compliant activation,
# and a dense layer at the end.
model = tf.keras.models.Sequential([
    tf.keras.layers.Input((32, 32, 3)),

    tf.keras.layers.Conv2D(32, 3, name='conv1', strides=2, use_bias=False),
    tf.keras.layers.BatchNormalization(momentum=0.9),
    tf.keras.layers.Activation(beatmup_keras.sigmoid_like),

    tf.keras.layers.Conv2D(64, 3, name='conv2', groups=4, use_bias=False),
    tf.keras.layers.BatchNormalization(momentum=0.9),
    tf.keras.layers.Activation(beatmup_keras.sigmoid_like),
    beatmup_keras.Shuffle(),

    tf.keras.layers.Conv2D(64, 3, name='conv3', groups=8, use_bias=False),
    tf.keras.layers.BatchNormalization(momentum=0.9),
    tf.keras.layers.Activation(beatmup_keras.sigmoid_like),
    beatmup_keras.Shuffle(),

    tf.keras.layers.Conv2D(96, 3, name='conv4', groups=8, use_bias=False),
    tf.keras.layers.BatchNormalization(momentum=0.9),
    tf.keras.layers.Activation(beatmup_keras.sigmoid_like),
    beatmup_keras.Shuffle(),

    tf.keras.layers.Conv2D(96, 3, name='conv5', groups=12, use_bias=False),
    tf.keras.layers.BatchNormalization(momentum=0.9),
    tf.keras.layers.Activation(beatmup_keras.sigmoid_like),
    beatmup_keras.Shuffle(),

    tf.keras.layers.Conv2D(144, 3, name='conv6', groups=12, use_bias=False),
    tf.keras.layers.BatchNormalization(momentum=0.9),
    tf.keras.layers.Activation(beatmup_keras.sigmoid_like),

    tf.keras.layers.GlobalAveragePooling2D(),

    tf.keras.layers.Dense(100, name='dense', use_bias=True),
])
model.summary()

# "Compile" the model for TensorFlow
model.compile(optimizer=tf.keras.optimizers.Adam(),
              loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
              metrics=[tf.keras.metrics.SparseCategoricalAccuracy()])


# Train the model
print('===== Training model...')
lr_scheduler = tf.keras.callbacks.LearningRateScheduler(lambda epoch, _: 0.1 if epoch < 20 else 0.01)
model.fit(train_images, train_labels, epochs=50, batch_size=256,
    validation_data=(test_images, test_labels),
    callbacks=[lr_scheduler])


# Save the model
model.save("cifar100_test")

# Load the model
model = tf.keras.models.load_model('cifar100_test')


# # #
# # #  The TF model is ready now. The Beatmup-related part starts here.
# # #

# Create a Beatmup model from the trained keras model
print('===== Converting model...')
ctx = beatmup.Context()
test_model, test_model_data = beatmup_keras.export_model(model, ctx)

# TF model is trained using categorical cross-entropy as loss function.
# From the inference standpoint, it is somehow equivalent to have a softmax layer at the end, in order to get classification probabilities.
# Add a Softmax layer to the converted model, and connect it to the last dense layer:
test_model.append(beatmup.nnets.Softmax('softmax'))
test_model.add_connection('dense', 'softmax')

# Initialize inference task
inference = beatmup.nnets.InferenceTask(test_model, test_model_data)

# Run the inference on CIFAR100 test set in a loop over all the images
print('===== Running inference on the test set...')
total = len(test_labels)    # total number of images
score = 0                   # number of correctly classified images
for i in range(total):
    # Get the image
    image = numpy.copy(test_images[i], 'C')
    image = beatmup.Bitmap(ctx, image)

    # Connect it to the model
    inference.connect(image, test_model.get_first_operation())

    # Run the inference: this is where Beatmup does the job
    ctx.perform_task(inference)

    # Get the class probabilities and the class label
    probabilities = test_model.get_last_operation().get_probabilities()
    prediction = numpy.argmax(probabilities)

    # Score a point if the prediction matches the ground truth
    if prediction == test_labels[i]:
        score +=1

    # Print something
    if i % 1000 == 0 and i > 0:
        print("  %4d/%d: %0.2f%%" % (i, total, score * 100.0 / i))

print('===== All good.')
print('Converted model top-1 test accuracy: %0.2f%%' % (score * 100.0 / total))

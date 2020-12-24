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
import argparse
import beatmup
import cv2
import numpy
import os


# parse arguments
parser = argparse.ArgumentParser(description='Upscales a given image or video.')
parser.add_argument('-c', '--codec', type=str, default='MJPG',
                    help='FourCC codec specification to encode the output video.')
parser.add_argument('-o', '--output', type=str, default=None,
                    help='Output filename.')
parser.add_argument('input', type=str, nargs=1,
                    help='Input video or image filename')
args = parser.parse_args()

# get input and output filename
input_filename = args.input[0]
output_filename = "%s_x2%s" % os.path.splitext(input_filename) if args.output is None else args.output

# initialize context
ctx = beatmup.Context()

# crete a resampler
resampler = beatmup.BitmapResampler(ctx)

# choose resampling algorithm
resampler.mode = beatmup.BitmapResampler.CONVNET

# try to read a frame
print(f"Trying to open {input_filename}")
try:
    reader = cv2.VideoCapture(input_filename)
    success, image = reader.read()
    assert success
except:
    print(f"Cannot read or decode {input_filename}")
    exit(-1)

# get the image size
h, w, c = image.shape
if c != 3:
    print(f"A 3-channel color input video expected, but {c} input channels got")
    exit(-1)

# set up the encoder
fourcc = cv2.VideoWriter_fourcc(*args.codec)
writer = cv2.VideoWriter(output_filename, fourcc, reader.get(cv2.CAP_PROP_FPS), (2 * w, 2 * h))

# set up output bitmap
resampler.output = beatmup.Bitmap(ctx, numpy.zeros((2 * h, 2 * w, 3), numpy.uint8))

# loop over input frames
print(f"Upsampling {w}x{h} to {2*w}x{2*h}...")
count = 0
while success:
    # feed the resampler with the image
    resampler.input = beatmup.Bitmap(ctx, image)

    # send it to the context
    ctx.submit_task(resampler)
        # This is unblocking: the output image is likely not ready right after this call!
        # Doing so to decode the next frame (below) at the same time with processing the current one.

    # read a frame
    success, image = reader.read()
    if not success:
        # with OpenCV sometimes you need to insist...
        for i in range(50):
            success, image = reader.read()
            if success: break
    
    # wait until the resampling task finishes
    ctx.wait()
        # Without this call a segfault is very likely to happen, because on the next iteration of the loop the task
        # input image will be modified while being processed.

    # write the processed image
    writer.write(numpy.array(resampler.output, copy=False))

    # clear GPU memory
    if count % 10 == 0:
        ctx.empty_gpu_recycle_bin()

    # print frame numbers
    print(count, "frames processed\r", end='')
    count += 1

# write the last frame
ctx.wait()
writer.write(numpy.array(resampler.output, copy=False))

print()
print("All good.")
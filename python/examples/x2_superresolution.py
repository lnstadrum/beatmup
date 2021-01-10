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
parser = argparse.ArgumentParser(description='Upscales a given image using a neural net.')
parser.add_argument('-o', '--output', type=str, default=None,
                    help='Output filename')
parser.add_argument('input', type=str, nargs=1,
                    help='Input image filename')
args = parser.parse_args()

# get input and output filenames
input_filename = args.input[0]
output_filename = "%s_x2%s" % os.path.splitext(input_filename) if args.output is None else args.output

# try to read the input image
image = None
try:
    image = cv2.imread(input_filename)
except:
    print(f"Cannot read or decode {input_filename}")
    exit(-1)

# initialize context
ctx = beatmup.Context()

# crete a resampler
resampler = beatmup.BitmapResampler(ctx)

# choose resampling algorithm
resampler.mode = beatmup.BitmapResampler.CONVNET

# get the input shape
h,w,c = image.shape
assert c == 3, "A three-channel input image is expected"

# set up output bitmap
output = beatmup.Bitmap(ctx, numpy.zeros((2 * h, 2 * w, c), numpy.uint8))

# feed the resampler with input and output
resampler.input = beatmup.Bitmap(ctx, image)
resampler.output = output

# run the resampling task
ctx.perform_task(resampler)

# store the result to file
result = cv2.imwrite(output_filename, numpy.array(output, copy=False))
if result:
    print("Result is written to", output_filename)
    exit(0)
else:
    print("Cannot write to", output_filename)
    exit(-1)
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

STRINGIFY(
        uniform sampler2D images[8];
        varying highp vec2 texCoord;

        void main() {
            
        lowp vec4 f[8];
        for (int i = 0; i < 8; ++i)
            f[i] = texture2D(images[i], texCoord);
    
        gl_FragColor = vec4(
            dot(vec4(-0.252126, -0.242144, 0.117084, -0.132702), f[0])
                + dot(vec4(0.127865, -0.252344, 0.322198, -0.155683), f[1])
                + dot(vec4(0.547305, 0.020524, -0.045916, -0.134462), f[2])
                + dot(vec4(0.345354, 0.372740, 0.123347, -0.403906), f[3])
                + dot(vec4(-0.300054, -0.023707, 0.572507, 0.263337), f[4])
                + dot(vec4(0.042556, 0.077427, 0.108896, -0.435867), f[5])
                + dot(vec4(0.406181, 0.061378, -0.400790, -0.030865), f[6])
                + dot(vec4(-0.274727, -0.217618, 0.194801, -0.543920), f[7]),
            dot(vec4(-0.017875, -0.355846, -0.046530, -0.396200), f[0])
                + dot(vec4(0.176187, -0.099441, -0.161924, 0.223951), f[1])
                + dot(vec4(0.010400, -0.050064, 0.420149, 0.173052), f[2])
                + dot(vec4(0.168248, -0.131145, 0.032333, -0.015751), f[3])
                + dot(vec4(-0.065950, -0.154427, 0.258832, 0.385409), f[4])
                + dot(vec4(0.225860, -0.343601, -0.062952, 0.027433), f[5])
                + dot(vec4(-0.176993, -0.117001, 0.540424, -0.022135), f[6])
                + dot(vec4(-0.312847, 0.215080, -0.159207, 0.102902), f[7]),
            dot(vec4(-0.052403, -0.306354, -0.267390, -0.040456), f[0])
                + dot(vec4(0.062542, 0.154981, -0.252139, 0.189715), f[1])
                + dot(vec4(0.072998, 0.218112, 0.404643, 0.240041), f[2])
                + dot(vec4(-0.042164, -0.169113, 0.070921, -0.101108), f[3])
                + dot(vec4(-0.006682, 0.373462, 0.020076, 0.109603), f[4])
                + dot(vec4(0.232229, -0.338240, -0.049583, 0.189561), f[5])
                + dot(vec4(0.046800, 0.010614, 0.480897, -0.314840), f[6])
                + dot(vec4(-0.256822, 0.272001, 0.083232, 0.236393), f[7]),
            dot(vec4(0.117436, -0.431948, 0.310359, 0.002164), f[0])
                + dot(vec4(-0.553898, -0.233355, -0.048573, -0.273020), f[1])
                + dot(vec4(-0.573837, -0.423906, 0.021357, -0.077188), f[2])
                + dot(vec4(-0.527228, 0.330722, 0.509363, -0.071118), f[3])
                + dot(vec4(-0.254311, 0.321716, 0.123364, -0.309190), f[4])
                + dot(vec4(0.234208, 0.033504, -0.139172, -0.006956), f[5])
                + dot(vec4(0.213465, -0.103781, -0.053797, -0.339927), f[6])
                + dot(vec4(0.074393, 0.725498, 0.013804, 0.020300), f[7])
        ) + vec4(0.017410, 0.022572, 0.054556, -0.031126);
    
        }
    )
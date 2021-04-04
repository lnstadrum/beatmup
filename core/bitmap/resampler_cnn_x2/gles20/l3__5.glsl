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
        f[0] = texture2D(images[0], texCoord);
        f[1] = texture2D(images[1], texCoord);
        f[2] = texture2D(images[2], texCoord);
        f[3] = texture2D(images[3], texCoord);
        f[4] = texture2D(images[4], texCoord);
        f[5] = texture2D(images[5], texCoord);
        f[6] = texture2D(images[6], texCoord);
        f[7] = texture2D(images[7], texCoord);

        gl_FragColor = vec4(
            dot(vec4(0.053259, -0.112037, -0.084111, -0.149046), f[0])
                + dot(vec4(-0.172362, 0.293199, -0.040350, 0.153782), f[1])
                + dot(vec4(0.355087, 0.007064, 0.362957, -0.691431), f[2])
                + dot(vec4(-0.125504, -0.339471, -0.125976, 0.000840), f[3])
                + dot(vec4(0.352941, 0.017458, 0.067334, -0.103171), f[4])
                + dot(vec4(-0.230846, -0.346597, -0.014139, -0.080798), f[5])
                + dot(vec4(-0.038493, 0.217314, -0.041173, 0.432711), f[6])
                + dot(vec4(0.262872, 0.247423, -0.176407, -0.073326), f[7]),
            dot(vec4(0.031750, -0.213146, -0.021746, 0.110896), f[0])
                + dot(vec4(-0.112883, 0.198841, -0.001079, 0.172485), f[1])
                + dot(vec4(0.489706, 0.308194, 0.038799, 0.186685), f[2])
                + dot(vec4(-0.545181, -0.249594, 0.148477, -0.086228), f[3])
                + dot(vec4(0.290387, 0.161785, -0.015383, -0.049266), f[4])
                + dot(vec4(0.005402, 0.102561, 0.209584, 0.008983), f[5])
                + dot(vec4(-0.110551, 0.454321, -0.323038, 0.244951), f[6])
                + dot(vec4(-0.104136, 0.308779, 0.251464, 0.057520), f[7]),
            dot(vec4(0.258932, 0.079620, 0.031104, 0.226165), f[0])
                + dot(vec4(0.192799, -0.035544, -0.190123, -0.104465), f[1])
                + dot(vec4(0.230408, -0.162102, -0.180104, -0.030632), f[2])
                + dot(vec4(-0.090977, -0.166879, 0.164576, -0.251363), f[3])
                + dot(vec4(0.018125, -0.072575, 0.049107, 0.054167), f[4])
                + dot(vec4(0.283078, 0.168029, -0.078216, -0.067408), f[5])
                + dot(vec4(0.012882, 0.100934, -0.040421, -0.004997), f[6])
                + dot(vec4(0.062229, 0.069396, -0.017565, 0.042032), f[7]),
            dot(vec4(-0.116323, -0.785883, 0.205909, 0.073174), f[0])
                + dot(vec4(-0.212455, -0.154290, -0.390948, 1.042846), f[1])
                + dot(vec4(0.492488, -0.200328, 0.543313, 0.860420), f[2])
                + dot(vec4(-0.154311, -0.166430, 0.018086, 0.182116), f[3])
                + dot(vec4(-0.343958, 0.271096, -0.217405, 0.187951), f[4])
                + dot(vec4(0.297018, 0.024377, -0.208669, -0.219234), f[5])
                + dot(vec4(-0.793007, -0.030600, -0.071312, -0.259475), f[6])
                + dot(vec4(0.303709, 0.005426, -0.245330, -0.024524), f[7])
        ) + vec4(-0.020420, 0.033188, 0.064994, -0.192451);
    
        }
    )
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
        beatmupInputImage image;
        varying highp vec2 texCoord;
        uniform highp vec2 d1;
        uniform highp vec2 d2;

        lowp float fetch(highp float x, highp float y) {
           return dot(texture2D(image, vec2(x, y)).rgb, vec3(0.299, 0.587, 0.114));
        }

        void main() {
            
        highp float
            x0 = texCoord.x - d2.x,
            x1 = texCoord.x - d1.x,
            x2 = texCoord.x,
            x3 = texCoord.x + d1.x,
            x4 = texCoord.x + d2.x,

            y0 = texCoord.y - d2.y,
            y1 = texCoord.y - d1.y,
            y2 = texCoord.y,
            y3 = texCoord.y + d1.y,
            y4 = texCoord.y + d2.y;
    
        lowp vec4 i[6];
        i[0] = vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0));
        i[1] = vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1));
        i[2] = vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2));
        i[3] = vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3));
        i[4] = vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3));
        i[5] = vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4));
        lowp float i6 = fetch(x4, y4);
    
        gl_FragColor = vec4(
            dot(i[0], vec4(0.005347, 0.004778, -0.005302, 0.003640)) + dot(i[1], vec4(0.004086, 0.011256, 0.002062, 0.013120)) +
            dot(i[2], vec4(0.033595, 0.049320, 0.010749, -0.006713)) + dot(i[3], vec4(0.013601, 0.044550, 0.038561, -0.025998)) +
            dot(i[4], vec4(0.025062, 0.057162, -0.628090, -0.103348)) + dot(i[5], vec4(-0.000700, 0.028268, 0.020608, 0.002258)) + i6 * 0.045168,
            dot(i[0], vec4(0.128036, -0.114671, -0.097701, 0.023224)) + dot(i[1], vec4(-0.031644, -0.216766, -0.335574, 0.243885)) +
            dot(i[2], vec4(0.003221, 0.051334, -0.037200, 0.344168)) + dot(i[3], vec4(0.102088, 0.008774, 0.025890, 0.067080)) +
            dot(i[4], vec4(-0.026139, -0.069850, -0.025997, -0.009489)) + dot(i[5], vec4(-0.044000, 0.037695, 0.013142, 0.029528)) + i6 * -0.024174,
            dot(i[0], vec4(-0.035849, -0.059748, 0.033917, 0.011995)) + dot(i[1], vec4(-0.018034, -0.098851, -0.050205, 0.139517)) +
            dot(i[2], vec4(0.063679, 0.017400, -0.059367, 0.194220)) + dot(i[3], vec4(0.327989, -0.114581, -0.079136, 0.006617)) +
            dot(i[4], vec4(0.246280, -0.049414, -0.349726, -0.021787)) + dot(i[5], vec4(0.033380, -0.003031, -0.207905, -0.060268)) + i6 * 0.131375,
            dot(i[0], vec4(0.032603, 0.003083, -0.044345, -0.017346)) + dot(i[1], vec4(0.015475, -0.010317, 0.017755, -0.061896)) +
            dot(i[2], vec4(0.017327, 0.086646, -0.040160, 0.106385)) + dot(i[3], vec4(-0.182860, -0.263582, 0.060346, 0.026906)) +
            dot(i[4], vec4(0.272079, 0.295033, -0.228662, -0.103040)) + dot(i[5], vec4(-0.114225, -0.071961, 0.148951, 0.092089)) + i6 * -0.035343
        ) + vec4(0.04768, 0.02194, 0.00047, -0.00430);
    
        }
    )
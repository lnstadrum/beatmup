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
            dot(i[0], vec4(0.094151, 0.028558, -0.055016, -0.055700)) + dot(i[1], vec4(-0.014320, -0.035812, 0.294502, 0.249755)) +
            dot(i[2], vec4(0.042912, -0.024795, -0.244312, -0.410410)) + dot(i[3], vec4(0.011302, 0.088539, 0.063416, 0.140843)) +
            dot(i[4], vec4(-0.023440, -0.153250, -0.036562, 0.006886)) + dot(i[5], vec4(0.026477, 0.070108, 0.020074, -0.069296)) + i6 * -0.024952,
            dot(i[0], vec4(-0.001558, 0.007953, -0.005058, 0.008436)) + dot(i[1], vec4(0.002002, 0.021489, -0.013238, 0.022834)) +
            dot(i[2], vec4(-0.037504, -0.012864, -0.002739, 0.042952)) + dot(i[3], vec4(-0.008841, 0.287401, -0.107039, -0.029071)) +
            dot(i[4], vec4(-0.005925, 0.035914, 0.324808, -0.119480)) + dot(i[5], vec4(-0.007585, 0.013874, -0.158980, -0.141671)) + i6 * 0.010981,
            dot(i[0], vec4(0.002181, 0.002280, -0.002251, 0.000642)) + dot(i[1], vec4(0.008016, -0.001168, -0.002224, -0.002400)) +
            dot(i[2], vec4(-0.001198, -0.010388, 0.006527, 0.006941)) + dot(i[3], vec4(0.002587, -0.007759, -0.011536, -0.003485)) +
            dot(i[4], vec4(-0.004608, -0.026047, -0.518676, 0.020610)) + dot(i[5], vec4(0.002261, -0.006069, 0.034012, 0.526197)) + i6 * -0.011475,
            dot(i[0], vec4(-0.002426, 0.019610, 0.062215, 0.033716)) + dot(i[1], vec4(0.002708, 0.023233, 0.012166, 0.058464)) +
            dot(i[2], vec4(0.054572, 0.017292, 0.064647, 0.099508)) + dot(i[3], vec4(0.100933, 0.100705, 0.051387, 0.008242)) +
            dot(i[4], vec4(0.048193, 0.078650, 0.105418, 0.015843)) + dot(i[5], vec4(-0.018338, -0.003021, -0.049115, 0.000757)) + i6 * -0.020554
        ) + vec4(0.00082, 0.00268, -0.00167, 0.01626);
    
        }
    )
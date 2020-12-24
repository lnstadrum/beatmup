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
            dot(i[0], vec4(-0.000482, 0.000169, -0.003803, 0.000461)) + dot(i[1], vec4(0.001091, 0.009488, -0.007875, 0.010956)) +
            dot(i[2], vec4(-0.009568, 0.000913, -0.009647, 0.025419)) + dot(i[3], vec4(-0.024990, 0.006437, 0.003896, 0.018830)) +
            dot(i[4], vec4(0.555726, -0.569396, 0.004360, -0.012654)) + dot(i[5], vec4(-0.009086, 0.016608, -0.001837, -0.010033)) + i6 * 0.005205,
            dot(i[0], vec4(-0.012912, -0.011937, -0.004789, 0.018469)) + dot(i[1], vec4(-0.008436, 0.023113, -0.021619, -0.019183)) +
            dot(i[2], vec4(0.025679, -0.000580, 0.036316, -0.044842)) + dot(i[3], vec4(-0.241364, 0.344428, -0.062766, 0.001454)) +
            dot(i[4], vec4(-0.017653, 0.013759, 0.058156, -0.015812)) + dot(i[5], vec4(0.000614, 0.003904, -0.004050, -0.027531)) + i6 * 0.015057,
            dot(i[0], vec4(0.008103, -0.003316, 0.054108, 0.051322)) + dot(i[1], vec4(-0.010830, -0.017249, -0.006392, -0.048804)) +
            dot(i[2], vec4(-0.018471, 0.012964, 0.056502, -0.292197)) + dot(i[3], vec4(-0.009987, 0.047334, -0.000223, -0.004199)) +
            dot(i[4], vec4(-0.183589, 0.286621, -0.046520, -0.022432)) + dot(i[5], vec4(0.025663, -0.040345, -0.069948, 0.048641)) + i6 * 0.040964,
            dot(i[0], vec4(-0.004655, 0.009565, -0.059361, -0.011843)) + dot(i[1], vec4(0.000665, 0.016066, -0.049909, 0.162656)) +
            dot(i[2], vec4(-0.010658, 0.036962, -0.044378, 0.168678)) + dot(i[3], vec4(-0.352570, 0.126882, -0.092685, 0.054538)) +
            dot(i[4], vec4(-0.205213, 0.323803, -0.052908, 0.044086)) + dot(i[5], vec4(-0.013345, 0.079366, -0.124239, 0.004550)) + i6 * -0.013119
        ) + vec4(-0.00081, 0.00110, 0.35140, 0.03827);
    
        }
    )
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
            dot(i[0], vec4(-0.006459, 0.004277, 0.001090, -0.003324)) + dot(i[1], vec4(-0.005299, 0.003932, 0.003425, -0.019857)) +
            dot(i[2], vec4(0.017060, -0.001349, 0.005909, -0.031518)) + dot(i[3], vec4(0.293027, -0.289846, 0.001131, 0.036883)) +
            dot(i[4], vec4(-0.083183, 0.376548, -0.391172, 0.082278)) + dot(i[5], vec4(-0.012365, 0.030459, 0.004952, -0.002403)) + i6 * -0.011199,
            dot(i[0], vec4(-0.019845, -0.022839, 0.018068, 0.034174)) + dot(i[1], vec4(-0.009819, 0.006453, -0.012695, 0.048508)) +
            dot(i[2], vec4(-0.107881, -0.000269, 0.001507, 0.150265)) + dot(i[3], vec4(-0.168724, -0.169217, 0.090339, -0.033633)) +
            dot(i[4], vec4(0.001386, -0.029517, -0.023794, 0.033391)) + dot(i[5], vec4(-0.006815, -0.011589, 0.044136, -0.043275)) + i6 * 0.025284,
            dot(i[0], vec4(-0.024965, -0.009929, 0.023906, 0.017318)) + dot(i[1], vec4(-0.013254, 0.006283, 0.020972, -0.091887)) +
            dot(i[2], vec4(-0.004105, 0.042009, 0.007650, 0.100814)) + dot(i[3], vec4(0.167543, -0.020018, -0.063949, -0.047208)) +
            dot(i[4], vec4(0.151145, -0.018928, 0.052436, 0.001268)) + dot(i[5], vec4(-0.000754, 0.112210, 0.039306, -0.007933)) + i6 * 0.001767,
            dot(i[0], vec4(-0.004240, -0.004544, 0.016805, -0.026787)) + dot(i[1], vec4(0.003890, 0.048980, 0.011499, 0.078100)) +
            dot(i[2], vec4(-0.014988, -0.051506, -0.030137, -0.107932)) + dot(i[3], vec4(-0.035592, 0.077206, 0.026030, -0.053555)) +
            dot(i[4], vec4(-0.249846, 0.361090, 0.127462, -0.026529)) + dot(i[5], vec4(0.040386, 0.017463, -0.068886, -0.001720)) + i6 * -0.009792
        ) + vec4(0.01146, 0.29067, 0.01385, -0.00007);
    
        }
    )
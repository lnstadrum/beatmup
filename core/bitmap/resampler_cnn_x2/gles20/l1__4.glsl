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
        uniform beatmupSampler image;
        varying highp vec2 texCoord;
        uniform highp vec2 d1;
        uniform highp vec2 d2;

        lowp float fetch(highp float x, highp float y) {
           return dot(beatmupTexture(image, vec2(x, y)).rgb, vec3(0.299, 0.587, 0.114));
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
    
        gl_FragColor = vec4(0.00177, 0.48300, -0.01603, 0.06719) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(-0.021569,-0.089825,0.072791,-0.003418,0.208471,-0.030909,0.114554,0.056365,0.044684,-0.015608,-0.270800,-0.092612,0.018396,0.017954,0.016317,0.022369) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(-0.008333,0.306702,-1.055250,1.388922,0.071240,-0.041818,-0.232543,-3.004621,0.088697,-0.076596,0.715151,1.792750,0.024310,-0.004317,-0.697793,0.037345) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(-0.255891,0.104869,-0.058897,-0.042603,-0.077314,0.011196,0.169345,0.157126,1.120376,0.036445,0.096499,-0.778929,-0.019171,-0.005554,0.081402,1.503542) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(0.046984,0.017515,0.019090,-0.015185,-6.927587,-0.021934,0.104157,-0.029137,-1.717293,-1.062646,0.065100,-0.118923,0.082508,-0.066571,-0.005385,-0.040295) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(-0.036159,-0.067775,-0.018181,0.008685,0.116420,0.011401,-0.017810,0.028768,-0.035113,0.213316,-0.124632,-0.087970,-1.037636,-0.084738,0.041052,0.057682) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(0.005950,0.011951,0.016378,-0.025258,0.004099,0.105412,0.316848,0.195970,0.035212,0.087481,0.138085,0.009814,-0.035006,-0.071513,-0.039603,0.001099) +
            vec4(0.017523, 0.099207, -0.027254, 0.014684) * fetch(x4, y4);
    
        }
    )
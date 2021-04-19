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
    
        gl_FragColor = vec4(0.08759, -1.63137, 0.13803, 0.02172) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(-0.089135,0.179288,0.346675,0.190945,0.493795,0.866382,0.347269,0.008884,-0.013070,-0.083137,0.138448,0.000924,-0.168045,0.348996,-0.037812,0.058842) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(0.027155,0.056065,0.163074,-3.340480,0.141645,0.459684,-11.911879,0.293256,0.077902,0.052543,-0.352389,-8.601992,-0.016567,0.247787,-0.684756,0.085900) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(0.165925,0.172076,0.227046,-4.176210,0.051597,0.037690,0.390008,0.589405,0.515130,0.112091,0.044051,-0.005122,-0.216341,0.070314,-0.127518,0.486193) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(-7.140024,-3.680979,0.156105,0.109899,0.371860,0.038789,0.073905,-0.086949,-0.049213,-0.072586,0.053943,-0.005531,0.204860,0.065010,0.022214,0.042066) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(0.182301,-4.329381,0.241423,0.202447,0.051311,0.105365,-0.026811,-0.008616,0.119077,0.037337,-0.011308,0.025937,-0.226923,-0.195428,-0.024894,-0.030566) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(-0.013059,0.217960,0.187904,0.138790,0.010301,0.135496,-0.028483,-0.062401,-0.004404,0.008421,0.029855,0.002529,-0.059053,0.180539,0.005622,-0.020351) +
            vec4(0.036360, 0.029663, -0.005030, -0.006526) * fetch(x4, y4);
    
        }
    )
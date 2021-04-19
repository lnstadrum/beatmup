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
    
        gl_FragColor = vec4(0.21346, 0.16951, 0.00405, 0.05409) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(-0.001980,0.008405,0.035402,0.072370,-0.009713,0.090497,0.016986,0.044854,-0.010718,0.001070,0.011826,0.009907,-0.195300,0.176257,-0.135208,0.022312) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(0.080122,0.027606,0.042766,-0.026292,0.038915,0.058164,0.065019,0.354404,-0.004830,0.000059,-0.016392,0.023233,-0.143468,0.088948,0.418506,-0.017725) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(0.109569,-0.006309,0.021109,0.037566,0.091500,0.035894,-0.016648,-0.140886,-0.033695,0.003458,0.016265,-0.053792,-0.008787,0.258528,-0.276929,1.105432) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(0.053485,-2.157261,0.082070,0.059446,-2.073906,0.264722,0.094288,0.077987,-1.464292,-0.067730,0.027633,-0.018817,-0.381364,-0.086101,0.317903,0.844790) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(-0.010982,-0.022314,0.625615,-0.053941,-0.018748,0.402050,0.132364,0.071382,0.062023,1.574309,0.073921,-0.013352,-1.464381,-4.350053,0.445930,0.187670) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(0.016195,-0.040327,-0.060821,-0.045482,-0.032376,0.028917,0.003707,-0.017757,0.003439,0.027675,-0.157063,0.005591,0.033569,-2.984518,0.240070,0.185170) +
            vec4(-0.008115, 0.011187, -0.002947, 0.255144) * fetch(x4, y4);
    
        }
    )
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
    
        gl_FragColor = vec4(-1.27182, 0.02103, 0.06169, 0.02553) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(-0.005065,0.020398,-0.051154,0.001751,0.018340,-0.025023,0.024814,-0.013510,0.052339,0.002618,-0.029617,-0.048287,0.053407,0.160885,0.176052,0.187503) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(0.023476,0.020869,-0.053991,-0.120176,-0.001818,0.025944,-0.049911,-0.031344,0.051230,-0.005704,0.170540,0.316830,0.167625,0.540263,-7.884494,-3.844006) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(-0.151230,-0.047635,-0.028494,-0.097853,0.004701,-0.002277,-0.043161,0.249058,0.118863,-0.074256,-0.055258,0.346494,-2.018597,0.057049,0.157084,0.138626) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(0.129623,2.149005,-0.643161,-0.015603,0.214236,-0.016112,-0.003564,-0.001233,-15.040040,0.338883,-0.033880,0.004320,0.080616,-0.258163,0.081775,-0.005680) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(0.020603,-0.208441,-0.145961,0.040398,0.284452,0.224924,-0.012998,0.005008,0.161928,0.326951,0.141706,0.009545,0.160534,0.200385,0.000431,0.097380) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(0.022754,-0.004227,-0.039109,0.009734,0.000079,0.022003,0.022526,0.013856,0.023811,-0.027445,-0.084215,-0.079067,0.021304,0.053854,-0.056083,0.027083) +
            vec4(0.023105, 0.002578, 0.041984, 0.022389) * fetch(x4, y4);
    
        }
    )
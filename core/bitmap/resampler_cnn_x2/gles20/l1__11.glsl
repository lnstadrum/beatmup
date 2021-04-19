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
    
        gl_FragColor = vec4(0.03335, -0.00956, 0.01197, 0.01758) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(-0.023606,-0.084224,0.297727,-0.201692,0.004926,-0.107197,0.087297,0.537621,-0.064231,-0.035433,0.182793,0.067571,-0.095217,0.146237,-0.201771,0.112652) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(0.017587,0.062008,-0.050854,1.345676,-0.000696,-0.066705,-0.504240,-0.651227,-0.083824,0.030367,-0.040655,0.017784,-0.018562,0.208512,1.227950,-1.205137) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(-1.421177,0.119428,-0.005118,-0.011291,0.524557,0.410615,0.443093,0.095125,0.061970,-0.201495,0.153435,-0.204794,0.050615,-0.028995,-0.492230,-1.159553) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(0.362634,-0.393522,0.042826,-0.014857,-1.253214,-0.617606,-0.222647,0.262480,-0.190264,0.763509,-0.258532,-0.001027,1.485306,-0.027046,0.126516,0.312864) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(-0.024983,0.092631,-0.034728,-0.024762,0.989210,0.117765,-0.108167,-0.059290,-0.261786,-1.335983,1.303369,0.433212,-0.124894,-0.106626,-0.074196,-0.038357) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(0.021839,-0.040165,0.044888,-0.031698,-0.314135,0.234848,0.175026,-0.017996,0.052623,0.235662,-0.468642,-0.301040,-0.001753,-0.069490,0.086335,-0.083451) +
            vec4(0.029764, 0.058024, 0.092488, 0.043209) * fetch(x4, y4);
    
        }
    )
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
    
        gl_FragColor = vec4(0.05201, -0.10461, -0.81887, -0.33572) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(0.032022,-0.080294,0.029243,-0.038670,-0.114294,-0.078201,0.020160,0.073933,0.027926,0.276727,-1.613889,0.175590,0.031808,0.002283,0.054188,-0.038361) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(0.024862,-0.084711,0.228198,-0.031008,-0.018578,-0.358179,1.581842,-2.466157,0.079912,-0.097493,-0.163050,2.026774,0.080512,-0.032891,0.049259,0.115437) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(0.004519,-0.025443,0.288039,-0.647956,0.020704,0.026526,-0.046204,-0.154300,-0.077502,-0.048020,0.071193,-0.072986,0.310063,0.165407,0.074582,-0.094515) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(0.320434,-0.166746,0.057178,-0.065782,0.196011,0.071513,-0.047328,0.012751,0.081357,-0.121779,0.014728,0.023233,0.007943,-8.134983,0.441461,-0.049771) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(0.281887,-0.037103,0.001031,0.006163,-0.032523,0.031382,0.033239,0.032291,-0.062520,-0.067674,-0.054151,0.083241,0.037934,0.099645,0.366732,0.150772) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(0.011537,-0.096823,-0.012667,-0.030176,-0.016685,-0.058529,0.060209,0.073807,0.033696,-0.027727,0.012700,-0.101360,0.034038,-0.056367,0.032502,0.006953) +
            vec4(0.020139, 0.012407, -0.009012, 0.125368) * fetch(x4, y4);
    
        }
    )
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
    
        gl_FragColor = vec4(-0.70236, -0.85035, -2.27745, 0.11053) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(0.075651,0.126427,-0.031489,-0.045995,-0.010961,-0.019162,-0.007380,-0.013463,-0.017599,0.018702,-0.008424,-0.002009,0.009485,-0.048307,0.187227,-0.146030) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(-0.047878,0.414068,-1.830493,1.844672,-0.002304,-0.101580,0.062853,0.126133,-0.015272,-0.107289,-0.150046,-0.074302,0.070292,-0.019367,-0.065660,-0.157207) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(-0.083220,-0.002882,0.069936,0.215652,-0.109011,0.028859,-0.127805,-0.120471,0.011040,-0.004278,-0.135572,2.963505,0.240755,-0.164025,0.038612,0.146818) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(-0.111900,-0.104169,0.038173,-0.027322,1.858524,-0.223346,-0.037457,0.086906,-0.059698,-0.001066,0.010351,-0.035119,-0.100268,-0.150690,0.170790,-0.020958) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(-0.057126,-0.036663,-0.028712,0.047071,0.056164,-1.760982,0.346453,0.042152,-0.138990,-0.043615,-0.008878,-0.001385,-0.018950,-0.007644,0.062878,-0.146226) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(0.016735,0.057840,-0.047541,-0.043985,0.066645,0.033690,0.213370,-0.153778,-0.005311,-0.002294,-0.007646,-0.005593,0.029582,-0.076404,0.182609,-0.084183) +
            vec4(0.046887, 0.057672, 0.022062, 0.084808) * fetch(x4, y4);
    
        }
    )
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
    
        gl_FragColor = vec4(-2.51153, 0.16051, 0.02753, -2.30583) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(-0.010448,-0.017690,-0.010752,-0.004836,0.122211,0.079415,0.095909,0.344241,-0.035444,-0.111940,-0.490825,-0.291554,0.014821,0.023363,-0.005780,-0.004028) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(-0.015209,-0.010248,0.050136,0.149524,0.321778,0.245050,0.113416,-8.244999,-0.113343,-0.093418,-0.266164,1.550589,0.021280,-0.018607,0.019606,0.018238) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(-0.024049,-0.008464,0.008542,0.040394,-3.994846,0.137488,0.276190,-0.058824,-0.361184,-0.025761,0.008835,-0.206044,0.021016,-0.010107,0.015425,-0.041360) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(2.761049,0.043146,-0.037855,-0.019369,-5.374341,1.009109,-0.117133,0.030610,-0.288026,-0.162498,-0.136680,-0.013704,-0.027601,-0.080268,0.014585,-0.019575) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(0.008557,0.104031,-0.008495,0.025179,0.051765,0.168641,0.229311,-0.071418,-0.135682,0.027277,-0.024990,-0.026269,-0.062036,2.905930,-0.121253,-0.000383) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(-0.008112,-0.021707,-0.022455,-0.025014,0.042757,0.000517,-0.112393,0.197820,-0.061473,-0.010062,0.024071,0.014761,-0.009480,-0.063557,-0.230200,-0.082152) +
            vec4(-0.020112, 0.042833, 0.025941, -0.009227) * fetch(x4, y4);
    
        }
    )
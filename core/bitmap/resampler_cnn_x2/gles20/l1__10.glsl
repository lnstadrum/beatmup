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
    
        gl_FragColor = vec4(-2.62488, -0.00024, -0.57288, -0.00039) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(-0.031084,0.015255,0.096002,-0.064340,-0.018774,0.385778,-0.139517,-0.061294,0.061153,0.039947,0.090410,-0.186633,0.070522,0.266394,-0.202753,-0.082783) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(-0.069042,-0.043441,0.084788,0.054262,-0.004437,0.387179,0.328533,-0.607527,-0.144904,0.042068,-0.137997,0.168128,0.014587,-0.039592,1.538332,-1.574014) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(0.298012,-0.011574,-0.053807,-0.059325,-0.110969,0.057967,0.093947,-0.561557,1.452848,-1.120168,-0.069146,-0.159305,0.000449,0.074902,0.039818,0.340928) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(0.027081,3.116374,0.130567,-0.027822,-1.217810,0.298920,0.077950,-0.194150,0.117902,0.099246,-0.104730,0.016827,-0.435162,0.002803,-0.000678,-0.003850) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(0.064229,0.047776,0.098088,0.025584,-0.416675,0.086293,1.143826,0.276083,0.063546,0.326130,-0.047654,-0.029619,0.001611,-0.014702,-0.001627,-0.034543) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(-0.000109,-0.028723,-0.019703,-0.063853,0.008633,-0.062076,0.448811,0.203927,-0.022649,0.046261,-0.024509,-0.100843,-0.000887,-0.003122,-0.000162,0.002002) +
            vec4(0.049008, -0.435025, 0.048031, 0.007097) * fetch(x4, y4);
    
        }
    )
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
    
        gl_FragColor = vec4(-1.81290, -0.17109, -2.29791, -0.00273) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(-0.051984,-0.027098,-0.021667,-0.048284,-0.002337,0.028635,0.021709,0.007712,-0.013144,-0.034294,0.019872,-0.046512,0.016239,-0.014110,-0.098983,0.002540) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(-0.067009,-0.027492,-0.023569,-0.018408,-0.022077,-0.013624,0.004643,0.108859,0.002194,-0.035030,0.046082,-0.023066,0.048124,-0.029756,0.296399,-0.329507) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(-0.019329,-0.032164,-0.109093,-0.002235,-0.008711,-0.006964,0.059104,0.011526,-0.027765,-0.031360,-0.055695,-0.056704,0.235680,-0.065641,-0.110750,0.162115) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(0.093974,0.072379,-0.047444,-0.045409,-1.697324,0.071791,0.024676,-0.014128,0.063236,-0.211304,-0.092591,-0.062432,-0.113373,-0.014936,-0.081428,0.405972) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(2.574539,0.131877,-0.034706,-0.064834,-0.001293,1.405629,0.058375,-0.044020,-0.092716,0.148394,2.915333,0.075504,-1.286338,2.052202,-1.469234,0.352734) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(0.091387,-0.256850,-0.073320,-0.028804,-0.034758,-0.022568,-0.193405,-0.034033,-0.068374,-0.033449,-0.073450,-0.084179,-0.210689,0.295132,-0.010243,0.014199) +
            vec4(-0.032071, 0.052579, 0.010987, -0.081644) * fetch(x4, y4);
    
        }
    )
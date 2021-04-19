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
    
        gl_FragColor = vec4(0.00680, 0.27524, 0.34524, -0.00466) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(0.142224,0.016848,0.050259,0.275115,0.067565,0.046811,0.009989,0.119246,0.040033,-0.099619,0.008900,0.089287,-0.009463,-0.253872,-0.488504,1.346971) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(-0.462139,0.032662,-0.016051,-0.288092,-0.004956,0.214853,0.028670,-0.450404,-0.046108,-0.061024,0.035140,-0.328331,0.055789,0.169048,-0.013428,1.056601) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(-0.477644,0.971857,0.168988,-0.155536,0.100500,0.117904,0.210212,-0.153824,0.147660,0.123898,-0.056347,-0.338467,-0.090016,-1.327514,-0.098306,-0.036990) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(0.339871,0.229385,-1.398099,-0.242433,-3.296234,0.141604,0.052965,0.075976,0.194141,0.251162,-0.053452,-0.039102,0.033655,-1.032364,0.413728,-0.091916) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(0.697400,-1.314899,0.807736,0.771919,0.075186,-0.164791,-0.017465,0.077566,-0.146470,-0.026388,0.206750,0.082849,-0.036539,0.129050,-0.028317,0.277647) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(0.227891,-0.510371,1.092604,-0.907964,0.055778,0.173258,-0.029146,0.146916,0.031205,-0.007603,-0.005818,0.033810,0.033790,-0.001139,0.141019,-0.215856) +
            vec4(-0.077287, 0.070505, 0.015703, 0.049114) * fetch(x4, y4);
    
        }
    )
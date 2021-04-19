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
    
        gl_FragColor = vec4(0.12103, 0.23801, 0.01670, -1.22224) +
            vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0)) * mat4(-0.000993,0.002601,0.010360,0.012087,-0.017644,0.002409,-0.005350,0.037653,0.011904,-0.019027,0.059759,0.025096,0.210622,-0.200085,-0.096617,0.177885) +
            vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1)) * mat4(-0.034907,-0.000816,-0.044780,0.051386,-0.001349,0.009095,-0.021533,0.009771,-0.020445,-0.010086,0.084321,-0.238242,-0.132340,0.069390,-0.015241,-0.006420) +
            vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2)) * mat4(0.015158,-0.004252,-0.000598,0.008735,-0.001068,0.008814,-0.001320,-0.035801,0.030098,-0.027490,0.041045,-0.018589,-0.181365,0.081053,-0.042383,-0.337644) +
            vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3)) * mat4(-0.108622,-0.107332,-0.024152,0.005698,-0.041553,-0.086950,0.019060,0.014976,0.663355,-0.125103,0.107968,-0.047833,2.362099,-0.243966,0.037704,-0.385938) +
            vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3)) * mat4(0.019908,-0.103819,0.602081,-0.000977,-0.056280,0.617331,-0.185371,0.022453,0.030963,-0.483591,0.036535,-0.035695,-10.943556,-0.304731,-0.068151,-0.052290) +
            vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4)) * mat4(0.002398,-0.011372,-0.002397,-0.016949,0.009540,-0.097884,-0.067719,-0.053466,-0.018879,0.061452,0.150130,0.002328,-1.589461,-0.367392,0.063945,0.101446) +
            vec4(-0.016549, 0.013990, 0.004929, 0.089848) * fetch(x4, y4);
    
        }
    )
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
        uniform sampler2D images[8];
        varying highp vec2 texCoord;

        void main() {
            
        mediump vec4 sum = vec4(0.206178, -0.018126, 0.144261, -0.137801);
    
        sum += texture2D(images[0], texCoord) * mat4(0.340077,0.012986,0.002155,0.128053,-0.337266,-0.038149,-0.189716,-0.252363,-0.363181,-0.441258,-0.181627,-0.130080,-0.052178,-0.204372,-0.155512,1.494118);
        sum += texture2D(images[1], texCoord) * mat4(-0.209341,0.278133,0.050068,0.162571,0.242205,-1.110752,-0.183262,0.425787,-0.021508,-1.361506,-0.170597,0.243897,0.285674,-0.034731,-0.397750,-1.443178);
        sum += texture2D(images[2], texCoord) * mat4(-0.041358,-0.194016,0.371885,0.060833,-0.079068,0.068964,0.020376,0.044668,-0.160903,-0.037149,-0.004740,0.155115,0.044737,0.605865,-0.396711,-0.371307);
        sum += texture2D(images[3], texCoord) * mat4(-0.331231,-0.063356,0.037188,0.024671,0.222539,0.016167,-0.199047,0.069411,-0.035082,-0.012949,-0.069771,0.057520,-0.378006,-0.068015,-0.169999,0.141307);
        sum += texture2D(images[4], texCoord) * mat4(0.208694,0.024387,-0.080705,-0.092614,-0.090661,0.377338,-0.021183,0.122276,-0.327657,0.137176,-0.279556,-0.416470,0.011951,0.181335,-0.087424,-0.249516);
        sum += texture2D(images[5], texCoord) * mat4(0.008807,0.105975,0.026921,-0.108943,0.015716,0.297650,-0.363542,-0.133707,-0.279067,0.205233,-1.152121,0.114844,-0.119870,0.274859,-0.146368,-0.163447);
        sum += texture2D(images[6], texCoord) * mat4(0.114139,0.139128,0.105819,-0.290090,-0.344049,-0.297957,-0.067678,-0.432136,-0.001405,0.594953,-0.259244,-0.381970,-0.936454,-0.210973,-0.037248,-0.228432);
        sum += texture2D(images[7], texCoord) * mat4(0.121381,-0.067453,-0.045882,0.292471,-0.010840,0.194419,0.135406,0.090386,-0.188734,0.442487,-0.298218,-0.002680,-0.059592,0.624798,-0.222801,0.044306);
        gl_FragColor = sum;
    
        }
    )
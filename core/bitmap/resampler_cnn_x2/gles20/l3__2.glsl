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
            
        mediump vec4 sum = vec4(0.081930, 0.120358, -0.145525, 0.031622);
    
        sum += texture2D(images[0], texCoord) * mat4(-0.159689,-0.030061,-0.002726,0.395019,-0.376668,0.087704,-0.077810,0.117339,-0.129527,-0.067883,0.067023,-0.629378,-0.115404,0.032709,-0.232842,-0.425835);
        sum += texture2D(images[1], texCoord) * mat4(-0.014806,-0.116383,0.053914,0.592667,0.266616,-0.938465,-0.204858,0.047040,-0.091438,-0.240006,0.120099,0.576610,-0.015974,0.171075,-0.006556,0.441097);
        sum += texture2D(images[2], texCoord) * mat4(0.019432,0.089828,-0.053870,-0.022783,-0.105768,-0.061975,0.028279,0.136736,0.061879,0.735314,-0.381545,-0.179813,-0.205650,-0.339608,-0.279505,-0.012321);
        sum += texture2D(images[3], texCoord) * mat4(0.095841,0.051884,-0.013387,0.004001,0.042910,0.008607,-0.196159,0.052646,-0.328103,0.088779,-0.218873,0.028997,0.323829,-0.029416,-0.028533,-0.010326);
        sum += texture2D(images[4], texCoord) * mat4(-0.067041,-0.014085,0.046815,0.051227,0.014889,-0.153508,-0.018026,0.072195,0.042483,-1.307369,-0.192571,-0.219326,-0.034150,-0.599437,-0.056636,-0.118674);
        sum += texture2D(images[5], texCoord) * mat4(0.024772,-0.112046,0.106700,-0.007890,0.063089,0.394104,-0.442915,-0.117887,-0.026019,0.252791,-0.281324,-0.099877,0.031506,0.082491,0.051279,0.196347);
        sum += texture2D(images[6], texCoord) * mat4(-0.037114,-0.057818,0.037845,0.067973,0.313319,-0.005139,-0.164149,0.137804,0.196340,-0.041517,-0.062119,0.319639,1.002909,-0.311419,0.014496,-0.757798);
        sum += texture2D(images[7], texCoord) * mat4(-0.035899,-0.036666,0.029782,-0.075727,-0.028525,-0.935522,-0.263211,-0.028278,-0.180192,-0.621467,0.295305,-0.264024,0.131710,-1.017440,-0.045513,0.736167);
        gl_FragColor = sum;
    
        }
    )
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
            
        mediump vec4 sum = vec4(-0.143974, 0.129822, 0.007462, -1.912489);
    
        sum += texture2D(images[0], texCoord) * mat4(-0.054715,0.139876,-0.103480,1.240505,-0.360853,-0.902970,0.195491,0.473056,-0.129030,-0.176216,0.041463,0.456359,-1.297127,-0.259475,-0.455351,0.845977);
        sum += texture2D(images[1], texCoord) * mat4(0.290724,-0.341261,0.304390,-1.086493,-0.018041,0.160152,-0.062883,-0.256594,0.157876,-0.272685,0.006609,-0.372259,-0.819273,0.089423,0.444207,0.941680);
        sum += texture2D(images[2], texCoord) * mat4(0.141597,0.004056,-0.159252,0.038627,-0.078803,-0.180448,-0.048369,-0.002029,-0.041641,-0.049141,0.159448,0.275612,-0.074305,-1.190387,0.250879,-0.209971);
        sum += texture2D(images[3], texCoord) * mat4(0.007640,-0.077561,-0.272518,-0.038711,0.197606,-0.068144,-0.052400,0.036364,0.074817,0.004074,-0.072470,-0.014017,-0.177867,0.701254,0.283195,-0.289139);
        sum += texture2D(images[4], texCoord) * mat4(-0.927356,-0.191490,0.022830,-0.163126,0.238716,0.047866,-0.394998,-0.375625,-0.012939,0.091461,-0.081925,-0.053822,-0.500817,0.127043,0.794399,0.305478);
        sum += texture2D(images[5], texCoord) * mat4(0.053690,-0.096710,-0.157567,0.183322,-0.647353,0.171503,-0.300473,-0.269643,-0.118353,0.059039,-0.489491,0.024664,0.057380,-1.412720,0.671397,0.901744);
        sum += texture2D(images[6], texCoord) * mat4(0.215520,-0.214457,-0.256192,-0.342396,-0.160441,0.360658,0.000200,-0.045428,-0.045597,0.186063,-0.333883,-0.133908,-0.267143,-0.021235,0.529059,0.068839);
        sum += texture2D(images[7], texCoord) * mat4(0.319489,-0.044316,-0.476471,-0.013250,-0.067110,0.153144,0.105858,-0.098055,-0.016467,-0.102739,0.162311,0.023035,-0.150250,0.102700,-0.329456,-0.214850);
        gl_FragColor = sum;
    
        }
    )
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
            
        mediump vec4 sum = vec4(0.292714, 0.102345, -0.612442, 0.139480);
    
        sum += texture2D(images[0], texCoord) * mat4(0.243995,0.083579,-0.021887,-0.442663,0.032167,0.072433,-0.014544,0.370772,-0.009226,0.311353,0.527441,0.658669,-0.156549,0.044782,-0.427944,-1.087501);
        sum += texture2D(images[1], texCoord) * mat4(0.131361,-0.059876,0.089169,-0.336182,0.002323,-0.570086,0.102247,-0.352145,0.316617,-0.913424,-0.126646,0.675033,-0.134524,-1.703220,0.246941,1.002981);
        sum += texture2D(images[2], texCoord) * mat4(0.002956,-0.393569,-0.093910,0.014813,-0.368153,-0.278596,0.010886,0.065438,0.091754,-0.580033,-0.031472,-0.129207,0.075036,0.354011,-0.116954,-0.118453);
        sum += texture2D(images[3], texCoord) * mat4(-0.293421,-0.033044,0.138627,0.207823,0.085157,-0.039530,-0.072443,0.006040,-0.146930,-0.368758,-0.119914,0.074817,-0.484242,-0.039929,-0.125076,0.016843);
        sum += texture2D(images[4], texCoord) * mat4(0.008556,-0.136921,0.013478,0.487618,-0.206862,-0.140686,0.020241,0.017490,0.529667,-0.292353,-0.165093,-0.227336,-0.111921,-0.255162,0.077848,-0.077834);
        sum += texture2D(images[5], texCoord) * mat4(0.058606,-0.075280,-0.002136,-0.094858,0.148659,0.034417,-0.359463,0.219998,0.056402,0.519925,-0.520056,-0.075257,0.101293,0.306439,-0.825092,0.047786);
        sum += texture2D(images[6], texCoord) * mat4(-0.039384,0.055867,0.164143,-0.066680,0.343033,-0.227716,-0.337694,-0.237103,0.034435,-0.021214,-0.320190,-0.022219,-0.025029,-0.236951,-0.155582,0.403007);
        sum += texture2D(images[7], texCoord) * mat4(0.059028,-0.026302,-0.067031,0.043293,0.277867,-0.188713,-0.135971,0.271548,0.144841,0.146772,-0.095137,-0.035878,0.099474,-0.559115,0.377119,-0.205046);
        gl_FragColor = sum;
    
        }
    )
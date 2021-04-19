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
            
        mediump vec4 sum = vec4(-0.275453, 0.006453, 0.130673, -0.041759);
    
        sum += texture2D(images[0], texCoord) * mat4(-0.092817,-0.262202,0.245622,0.795693,-0.155614,0.204832,-0.260413,-0.414593,-0.099770,0.142595,-0.069853,0.323335,-0.019192,0.021404,0.009176,0.403129);
        sum += texture2D(images[1], texCoord) * mat4(-0.181076,-0.018183,-0.013210,-0.711067,0.142907,-1.050550,0.039227,0.427149,-0.139330,-0.163606,-0.719481,-0.315952,0.057384,-0.023934,-0.035451,0.650445);
        sum += texture2D(images[2], texCoord) * mat4(-0.004336,0.380400,-0.007062,-0.042813,0.067652,0.063110,0.066941,0.099539,0.034265,0.366354,-0.002633,-0.002930,0.004102,0.011122,-0.063391,-0.003352);
        sum += texture2D(images[3], texCoord) * mat4(0.232721,0.092258,-0.048268,0.022033,0.069036,-0.044679,0.026402,0.035267,-0.328344,-0.020407,0.060973,-0.082098,0.032349,-0.004681,0.000914,-0.008575);
        sum += texture2D(images[4], texCoord) * mat4(0.022608,0.221902,-0.119972,0.078487,-0.140954,-0.002499,0.061206,-0.116346,-0.051660,0.039605,0.049101,0.081080,-0.022284,-0.004782,-0.002426,-0.008417);
        sum += texture2D(images[5], texCoord) * mat4(-0.228879,0.131226,-0.128516,-0.125672,0.116898,-0.023247,-0.254885,-0.048021,0.078682,-0.029388,-0.793624,0.012872,-0.019310,0.000216,-0.019680,0.020511);
        sum += texture2D(images[6], texCoord) * mat4(-0.221635,0.204660,-0.692134,0.178934,0.323970,-0.110115,-0.281358,-0.022299,-0.069334,0.002180,-0.267172,-0.101215,-0.006488,-0.011440,-0.012374,0.039206);
        sum += texture2D(images[7], texCoord) * mat4(-0.174199,-0.191543,0.379362,-0.183127,-0.029768,-0.281950,-0.249720,0.010807,0.011879,0.017899,-0.101904,0.056715,-0.006864,0.026076,0.001964,-0.030276);
        gl_FragColor = sum;
    
        }
    )
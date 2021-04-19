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
            
        mediump vec4 sum = vec4(-0.777585, -0.171533, -0.157531, -0.128464);
    
        sum += texture2D(images[0], texCoord) * mat4(0.877749,-0.417934,0.488598,-2.349433,0.024799,-0.134086,-0.100873,-1.028196,-0.096243,-0.042697,0.065832,0.850636,0.056862,-0.598675,-2.298410,0.010682);
        sum += texture2D(images[1], texCoord) * mat4(-0.557405,0.371096,0.634930,-3.018342,-0.008089,-0.005554,0.008672,0.981144,0.071592,0.077072,-0.073111,-0.730503,-0.215953,-3.460881,-0.072014,0.035777);
        sum += texture2D(images[2], texCoord) * mat4(-0.044135,-0.259311,-0.646963,0.075349,-0.030482,0.176840,-0.068345,-0.051291,-0.064999,0.033458,0.019260,-0.015363,-0.031595,-0.121612,-0.058116,0.008611);
        sum += texture2D(images[3], texCoord) * mat4(-0.316309,-0.176758,0.552900,1.123340,0.305759,0.094437,-0.016936,0.003709,0.314558,0.079398,-0.018650,0.028886,0.276172,-0.065178,-0.020839,-0.036951);
        sum += texture2D(images[4], texCoord) * mat4(0.477525,-0.025481,-0.611786,-0.485637,-0.101071,0.080406,-0.108456,0.164161,0.014243,0.235167,-0.056712,0.204776,-1.380003,0.085917,-0.413198,-1.092390);
        sum += texture2D(images[5], texCoord) * mat4(0.648327,0.432689,-0.520623,-0.722618,-0.192273,0.297280,-0.025948,0.038755,-0.123718,0.272857,0.047971,-0.142547,-0.833254,0.027404,-1.242543,0.364771);
        sum += texture2D(images[6], texCoord) * mat4(-0.116084,0.153554,-1.309151,-0.337194,-0.099238,0.103837,-0.143357,-0.028995,-0.008711,0.178292,-0.594058,-0.224114,-0.172938,0.312399,0.232243,-0.076795);
        sum += texture2D(images[7], texCoord) * mat4(-0.101037,-0.341251,-0.303487,-0.061473,-0.096822,0.054887,0.047304,-0.020771,-0.207793,-0.165513,0.045815,0.314986,-0.043108,0.031815,0.137112,-0.103813);
        gl_FragColor = sum;
    
        }
    )
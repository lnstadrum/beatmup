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
        uniform sampler2D images[4];
        varying highp vec2 texCoord;

        void main() {
            
        lowp vec4 f[4];
        f[0] = texture2D(images[0], texCoord);
        f[1] = texture2D(images[1], texCoord);
        f[2] = texture2D(images[2], texCoord);
        f[3] = texture2D(images[3], texCoord);
    
        gl_FragColor = vec4(
            dot(vec4(-0.128480, -0.331011, 0.479560, -0.010360), f[0])
                + dot(vec4(-0.288763, -0.163952, -0.107318, 0.416246), f[1])
                + dot(vec4(-0.111557, 0.112956, -0.521888, 0.357350), f[2])
                + dot(vec4(0.530606, 0.321276, -0.207054, 0.483943), f[3]),
            dot(vec4(-0.164898, -0.142339, 0.054280, 0.323241), f[0])
                + dot(vec4(0.457874, -0.204089, -0.129695, 0.571394), f[1])
                + dot(vec4(0.215299, -0.149313, -0.000150, -0.533151), f[2])
                + dot(vec4(-0.348615, 0.161753, -0.035195, 0.245302), f[3]),
            dot(vec4(0.471591, 0.245929, -0.160316, -0.030097), f[0])
                + dot(vec4(-0.238465, 0.593387, -0.079710, 0.033316), f[1])
                + dot(vec4(0.015486, 0.092310, -0.267969, 0.129855), f[2])
                + dot(vec4(-0.013349, 0.616188, 0.280437, 0.119231), f[3]),
            dot(vec4(0.104719, 0.340025, -0.189702, 0.320893), f[0])
                + dot(vec4(-0.259299, -0.156502, 0.393652, -0.257146), f[1])
                + dot(vec4(0.036859, -0.127569, 0.089679, -0.169039), f[2])
                + dot(vec4(0.263064, 0.270407, -0.136138, 0.580510), f[3])
        ) + vec4(0.066900, 0.040832, -0.030687, 0.022122);
    
        }
    )
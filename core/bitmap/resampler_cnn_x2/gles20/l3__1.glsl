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
            
        lowp vec4 f[8];
        for (int i = 0; i < 8; ++i)
            f[i] = texture2D(images[i], texCoord);
    
        gl_FragColor = vec4(
            dot(vec4(0.055876, 0.177210, -0.051522, 0.283873), f[0])
                + dot(vec4(-0.116973, 0.281799, 0.055905, -0.128258), f[1])
                + dot(vec4(0.009184, 0.125242, 0.015966, -0.010202), f[2])
                + dot(vec4(-0.070879, -0.156507, 0.115037, -0.097470), f[3])
                + dot(vec4(0.245747, -0.135955, -0.049770, 0.188307), f[4])
                + dot(vec4(-0.391254, 0.039058, 0.102572, -0.034395), f[5])
                + dot(vec4(0.083545, 0.161361, -0.028600, -0.000068), f[6])
                + dot(vec4(0.231604, 0.175694, 0.178903, -0.082046), f[7]),
            dot(vec4(0.515885, 0.190248, -0.097944, -0.067832), f[0])
                + dot(vec4(-0.224788, 0.266262, 0.016807, -0.162557), f[1])
                + dot(vec4(-0.246043, -0.148687, 0.201186, -0.045879), f[2])
                + dot(vec4(0.490289, 0.074054, -0.361585, -0.183870), f[3])
                + dot(vec4(0.272062, 0.002080, -0.028351, -0.287422), f[4])
                + dot(vec4(-0.187889, -0.130660, -0.181960, 0.086034), f[5])
                + dot(vec4(0.201469, -0.270017, 0.245712, -0.097141), f[6])
                + dot(vec4(-0.003175, 0.039137, 0.005032, 0.139614), f[7]),
            dot(vec4(-0.165958, -0.130224, 0.011347, 0.050711), f[0])
                + dot(vec4(0.059367, -0.222354, -0.116148, 0.206650), f[1])
                + dot(vec4(0.168267, 0.473339, -0.200884, 0.001075), f[2])
                + dot(vec4(-0.260489, 0.099904, 0.178306, 0.123270), f[3])
                + dot(vec4(0.219992, 0.041969, 0.075181, 0.231194), f[4])
                + dot(vec4(0.130405, 0.159468, 0.476651, -0.237148), f[5])
                + dot(vec4(-0.221953, -0.108773, -0.150325, 0.070254), f[6])
                + dot(vec4(-0.149652, 0.216296, -0.125102, -0.142614), f[7]),
            dot(vec4(-0.132434, -0.318489, 0.172630, -0.258627), f[0])
                + dot(vec4(0.030739, 0.147420, -0.227831, 0.160869), f[1])
                + dot(vec4(-0.149877, 0.292166, -0.252180, -0.242255), f[2])
                + dot(vec4(0.051046, 0.237812, -0.133260, -0.254408), f[3])
                + dot(vec4(-0.134048, -0.214093, 0.052929, 0.028543), f[4])
                + dot(vec4(0.016680, 0.211384, -0.050921, -0.103808), f[5])
                + dot(vec4(-0.132501, -0.258967, 0.151876, -0.246464), f[6])
                + dot(vec4(0.069683, 0.135605, 0.187209, -0.294939), f[7])
        ) + vec4(-0.107753, 0.068446, 0.025846, -0.032269);
    
        }
    )
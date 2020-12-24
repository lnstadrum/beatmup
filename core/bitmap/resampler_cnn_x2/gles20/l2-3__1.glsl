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
        uniform sampler2D images[3];
        varying highp vec2 texCoord;
        uniform highp vec2 d1;

        lowp vec4 fetch(sampler2D image, highp float x, highp float y) {
            return texture2D(image, vec2(x, y));
        }

        void main() {
            
        highp float
            x0 = texCoord.x - d1.x,
            x1 = texCoord.x,
            x2 = texCoord.x + d1.x,

            y0 = texCoord.y - d1.y,
            y1 = texCoord.y,
            y2 = texCoord.y + d1.y;
            
        lowp vec4 i0, i1, i2, i3, i4, i5, i6, i7, i8;
        mediump vec4 sum;
    
            i0 = fetch(images[0], x0, y0);
            i1 = fetch(images[0], x1, y0);
            i2 = fetch(images[0], x2, y0);
            i3 = fetch(images[0], x0, y1);
            i4 = fetch(images[0], x1, y1);
            i5 = fetch(images[0], x2, y1);
            i6 = fetch(images[0], x0, y2);
            i7 = fetch(images[0], x1, y2);
            i8 = fetch(images[0], x2, y2);
        
            sum = vec4(
                dot(vec4(0.150679, -0.019474, -0.029123, -0.142624), i0)
                + dot(vec4(0.287332, -0.087410, -0.036469, 0.044613), i1)
                + dot(vec4(-0.044255, 0.022512, -0.104204, -0.005517), i2)
                + dot(vec4(0.034644, -0.025179, -0.115863, -0.306365), i3)
                + dot(vec4(-0.115385, -0.500529, 0.023296, -0.145068), i4)
                + dot(vec4(-0.128211, 0.188752, 0.086284, 0.014691), i5)
                + dot(vec4(0.150196, 0.192363, 0.033905, -0.227925), i6)
                + dot(vec4(-0.210397, 0.296139, -0.125498, -0.002859), i7)
                + dot(vec4(0.037247, -0.103270, 0.194529, -0.245874), i8),
                dot(vec4(0.055519, -0.050356, -0.180452, 0.039056), i0)
                + dot(vec4(0.046434, -0.069571, -0.026821, 0.049788), i1)
                + dot(vec4(-0.224423, 0.073293, -0.019265, 0.082499), i2)
                + dot(vec4(0.061881, -0.103045, -0.048860, -0.036646), i3)
                + dot(vec4(0.270238, 0.129781, 0.211513, 0.028964), i4)
                + dot(vec4(-0.184625, 0.115876, 0.037878, 0.132366), i5)
                + dot(vec4(0.104987, -0.040608, 0.233406, -0.032626), i6)
                + dot(vec4(0.054374, -0.157855, -0.101248, -0.012816), i7)
                + dot(vec4(0.015197, 0.071781, -0.059980, 0.103137), i8),
                dot(vec4(-0.228277, -0.151248, 0.001040, -0.142406), i0)
                + dot(vec4(0.014321, -0.174348, 0.121265, 0.075219), i1)
                + dot(vec4(-0.210315, -0.059768, -0.052644, 0.007565), i2)
                + dot(vec4(-0.174409, 0.080198, -0.181451, 0.072733), i3)
                + dot(vec4(-0.011682, 0.040221, -0.112104, 0.007618), i4)
                + dot(vec4(-0.009939, -0.012439, -0.123723, -0.194466), i5)
                + dot(vec4(0.067171, -0.168868, -0.010676, -0.007496), i6)
                + dot(vec4(-0.125278, -0.042862, -0.035566, 0.102626), i7)
                + dot(vec4(0.122089, 0.138603, -0.164123, 0.006489), i8),
                dot(vec4(0.154779, -0.054307, 0.175308, -0.190052), i0)
                + dot(vec4(-0.080150, -0.205862, 0.211016, -0.110570), i1)
                + dot(vec4(-0.090483, -0.162126, 0.160360, -0.001270), i2)
                + dot(vec4(-0.019729, -0.025541, -0.306895, -0.088397), i3)
                + dot(vec4(-0.027187, 0.304914, -0.337636, -0.133929), i4)
                + dot(vec4(-0.145654, 0.225889, -0.171116, -0.122482), i5)
                + dot(vec4(-0.047806, 0.031370, 0.036539, 0.004099), i6)
                + dot(vec4(0.009619, -0.013805, 0.361792, 0.205328), i7)
                + dot(vec4(0.080941, 0.132490, -0.052255, -0.093758), i8)
            );
        
            i0 = fetch(images[1], x0, y0);
            i1 = fetch(images[1], x1, y0);
            i2 = fetch(images[1], x2, y0);
            i3 = fetch(images[1], x0, y1);
            i4 = fetch(images[1], x1, y1);
            i5 = fetch(images[1], x2, y1);
            i6 = fetch(images[1], x0, y2);
            i7 = fetch(images[1], x1, y2);
            i8 = fetch(images[1], x2, y2);
        
            sum += vec4(
                dot(vec4(0.242339, 0.166414, -0.012915, 0.026027), i0)
                + dot(vec4(0.776090, -0.037953, 0.284355, 0.061055), i1)
                + dot(vec4(-0.141011, 0.049654, -0.155843, 0.001682), i2)
                + dot(vec4(-0.141859, -0.280229, 0.294881, -0.082962), i3)
                + dot(vec4(-1.091597, 0.119809, 0.023661, -0.031465), i4)
                + dot(vec4(-0.286269, -0.039551, -0.163213, 0.177392), i5)
                + dot(vec4(-0.052735, 0.214080, 0.062707, -0.026204), i6)
                + dot(vec4(0.010187, 0.343099, -0.125262, 0.174939), i7)
                + dot(vec4(-0.011633, 0.154403, 0.003961, -0.077377), i8),
                dot(vec4(0.158478, -0.052340, -0.123616, -0.105471), i0)
                + dot(vec4(0.381524, -0.056287, 0.313448, -0.060076), i1)
                + dot(vec4(-0.111661, 0.094473, -0.483787, -0.167255), i2)
                + dot(vec4(0.331459, 0.027219, 0.156673, 0.090647), i3)
                + dot(vec4(0.850490, 0.061220, 0.305707, -0.001777), i4)
                + dot(vec4(0.208795, -0.100186, -0.009836, 0.370872), i5)
                + dot(vec4(0.147190, 0.121820, -0.025906, 0.022474), i6)
                + dot(vec4(0.091807, -0.090571, 0.179601, -0.226579), i7)
                + dot(vec4(-0.035697, 0.028123, -0.028821, 0.035508), i8),
                dot(vec4(-0.015246, 0.053336, -0.029879, 0.056119), i0)
                + dot(vec4(0.062607, -0.079441, -0.104740, 0.099157), i1)
                + dot(vec4(-0.195604, -0.116540, -0.170626, -0.163014), i2)
                + dot(vec4(-0.028532, -0.098573, -0.151831, -0.057282), i3)
                + dot(vec4(0.058806, 0.102043, 0.059905, -0.033338), i4)
                + dot(vec4(0.048204, 0.138363, -0.010455, -0.119900), i5)
                + dot(vec4(0.002422, -0.164269, 0.021085, 0.131500), i6)
                + dot(vec4(0.093611, -0.157705, -0.085742, -0.032844), i7)
                + dot(vec4(-0.075160, -0.173757, -0.250685, -0.041044), i8),
                dot(vec4(-0.093101, -0.164919, -0.115989, -0.088928), i0)
                + dot(vec4(-0.012891, 0.053240, 0.118751, 0.274329), i1)
                + dot(vec4(0.229345, -0.025838, 0.187503, 0.162843), i2)
                + dot(vec4(-0.074431, 0.060996, -0.031656, -0.097112), i3)
                + dot(vec4(-0.280453, -0.041519, 0.016041, -0.281182), i4)
                + dot(vec4(-0.257276, -0.072397, -0.104060, 0.012655), i5)
                + dot(vec4(0.012875, 0.059341, 0.008095, 0.098960), i6)
                + dot(vec4(0.099937, -0.017408, -0.193123, 0.274848), i7)
                + dot(vec4(0.019295, 0.113864, 0.071755, -0.138528), i8)
            );
        
            i0 = fetch(images[2], x0, y0);
            i1 = fetch(images[2], x1, y0);
            i2 = fetch(images[2], x2, y0);
            i3 = fetch(images[2], x0, y1);
            i4 = fetch(images[2], x1, y1);
            i5 = fetch(images[2], x2, y1);
            i6 = fetch(images[2], x0, y2);
            i7 = fetch(images[2], x1, y2);
            i8 = fetch(images[2], x2, y2);
        
            sum += vec4(
                dot(vec4(-0.001437, -0.031542, -0.040035, 0.277247), i0)
                + dot(vec4(-0.047803, -0.123038, 0.059905, 0.189308), i1)
                + dot(vec4(-0.035479, 0.031500, 0.097836, 0.061726), i2)
                + dot(vec4(-0.080977, -0.213202, 0.018444, 0.304693), i3)
                + dot(vec4(0.109539, 0.151218, 0.132794, 0.211192), i4)
                + dot(vec4(-0.102710, -0.366637, 0.288329, 0.074144), i5)
                + dot(vec4(-0.075727, -0.180477, 0.033536, -0.136574), i6)
                + dot(vec4(0.100936, -0.232561, 0.168407, -0.097884), i7)
                + dot(vec4(0.313787, -0.411939, 0.180746, 0.123170), i8),
                dot(vec4(0.019106, 0.073380, -0.098528, 0.019988), i0)
                + dot(vec4(-0.145829, -0.106199, 0.174058, 0.016830), i1)
                + dot(vec4(-0.054284, -0.103699, 0.067494, 0.020628), i2)
                + dot(vec4(-0.005955, 0.162760, 0.076300, 0.189369), i3)
                + dot(vec4(0.083016, 0.147119, -0.085446, 0.085316), i4)
                + dot(vec4(0.076689, 0.015764, -0.063085, -0.023842), i5)
                + dot(vec4(-0.134366, 0.140358, -0.078940, 0.219639), i6)
                + dot(vec4(0.177913, 0.290410, -0.103363, 0.235954), i7)
                + dot(vec4(-0.115001, 0.352512, -0.247720, 0.088736), i8),
                dot(vec4(-0.125993, -0.075337, 0.005328, -0.089030), i0)
                + dot(vec4(0.108073, -0.201523, -0.133912, 0.011901), i1)
                + dot(vec4(-0.006833, -0.109998, -0.167452, 0.034895), i2)
                + dot(vec4(-0.153863, -0.108023, -0.015898, -0.114943), i3)
                + dot(vec4(-0.077247, 0.004138, -0.047116, 0.024241), i4)
                + dot(vec4(0.080690, -0.213338, -0.186856, -0.130180), i5)
                + dot(vec4(-0.150198, -0.027175, -0.034824, -0.102772), i6)
                + dot(vec4(0.008385, 0.031112, -0.241129, -0.088333), i7)
                + dot(vec4(-0.117406, -0.121856, -0.111575, -0.046878), i8),
                dot(vec4(-0.016147, 0.093210, -0.033255, 0.336574), i0)
                + dot(vec4(-0.100425, 0.148216, -0.030448, 0.801636), i1)
                + dot(vec4(-0.061143, 0.708951, 0.039205, 0.376718), i2)
                + dot(vec4(-0.119254, -0.068057, 0.065241, 0.114077), i3)
                + dot(vec4(0.489678, -0.074146, -0.126485, 0.383722), i4)
                + dot(vec4(0.503597, -0.373839, 0.201851, 0.452352), i5)
                + dot(vec4(-0.084863, -0.038751, -0.030304, -0.148421), i6)
                + dot(vec4(-0.151030, -0.064972, 0.263671, 0.008642), i7)
                + dot(vec4(-0.261259, -0.119514, -0.189481, -0.134665), i8)
            );
        
        gl_FragColor = sum + vec4(0.129449, 0.105419, -0.047723, -0.026679);
    
        }
    )

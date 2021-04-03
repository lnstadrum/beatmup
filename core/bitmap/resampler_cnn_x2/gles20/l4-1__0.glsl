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
        mediump vec4 sum = vec4(-0.005543, 0.035343, 0.111945, 0.188876);
    
            i0 = fetch(images[0], x0, y0);
            i1 = fetch(images[0], x1, y0);
            i2 = fetch(images[0], x2, y0);
            i3 = fetch(images[0], x0, y1);
            i4 = fetch(images[0], x1, y1);
            i5 = fetch(images[0], x2, y1);
            i6 = fetch(images[0], x0, y2);
            i7 = fetch(images[0], x1, y2);
            i8 = fetch(images[0], x2, y2);
        
            sum += i0 * mat4(0.056841, -0.079847, 0.107705, -0.118157, 0.484930, -0.273643, 0.003103, -0.045441, 0.295203, -0.063767, -0.126509, -0.175568, -0.109938, 0.083481, -0.137112, -0.140397);
            sum += i1 * mat4(0.168945, 0.152012, 0.048327, -0.033125, -0.137039, -0.051527, 0.333625, -0.124664, -0.083109, 0.317708, 0.155900, -0.083961, -0.020126, 0.294009, -0.061646, -0.097599);
            sum += i2 * mat4(0.005214, -0.005649, 0.112761, -0.102735, -0.252076, 0.239204, -0.073468, 0.225580, 0.064357, 0.025578, 0.050532, 0.006366, 0.211294, -0.198395, 0.064650, -0.128495);
            sum += i3 * mat4(-0.164095, -0.189549, -0.080176, -0.028413, 0.616564, -0.596019, -0.227013, -0.211008, -0.078681, -0.051696, -0.100579, -0.051345, -0.151022, 0.128887, -0.011424, -0.045995);
            sum += i4 * mat4(0.047598, 0.017669, -0.177137, 0.178763, -0.448698, -0.131035, 0.192643, 0.462104, -0.099411, -0.628146, -0.012981, -0.105168, -0.128617, -0.145315, -0.154138, 0.028123);
            sum += i5 * mat4(-0.045235, 0.125064, -0.155752, 0.080453, -0.369439, 0.322312, -0.020799, 0.243836, -0.314208, 0.243857, 0.019348, 0.235568, 0.001802, -0.147397, -0.004394, 0.102641);
            sum += i6 * mat4(-0.029831, -0.062240, -0.090542, -0.060308, -0.096807, 0.021109, -0.114323, 0.130641, 0.095398, -0.074484, -0.077982, 0.018375, 0.106235, -0.048648, 0.036626, -0.012677);
            sum += i7 * mat4(-0.004731, 0.144423, 0.020892, 0.005438, 0.463751, -0.090464, 0.100464, 0.143822, 0.310329, 0.035630, 0.055798, 0.103435, 0.025497, 0.041939, 0.026833, 0.044885);
            sum += i8 * mat4(-0.139627, -0.170073, -0.179924, -0.078809, -0.121892, 0.319786, -0.042095, 0.040828, 0.084096, 0.011838, -0.093573, 0.118889, 0.051062, -0.087721, 0.035846, 0.015253);
        
            i0 = fetch(images[1], x0, y0);
            i1 = fetch(images[1], x1, y0);
            i2 = fetch(images[1], x2, y0);
            i3 = fetch(images[1], x0, y1);
            i4 = fetch(images[1], x1, y1);
            i5 = fetch(images[1], x2, y1);
            i6 = fetch(images[1], x0, y2);
            i7 = fetch(images[1], x1, y2);
            i8 = fetch(images[1], x2, y2);
        
            sum += i0 * mat4(-0.029082, -0.072009, -0.169456, 0.096057, 0.025831, 0.052348, 0.053294, 0.280659, 0.268453, 0.155196, -0.120324, 0.210454, 0.104274, -0.103469, 0.108721, 0.029714);
            sum += i1 * mat4(0.071445, -0.030023, -0.150032, 0.053829, 0.084044, 0.181592, -0.153502, 0.917490, 0.308620, -0.011248, 0.061879, -0.540648, 0.389142, -0.008075, 0.038591, -0.991965);
            sum += i2 * mat4(-0.241254, 0.155034, -0.092045, 0.048615, 0.208588, -0.136647, 0.123846, -0.333292, -0.091957, -0.085904, -0.108662, -0.138297, -0.146270, 0.097054, -0.159370, 0.133366);
            sum += i3 * mat4(-0.151522, -0.071867, 0.161772, -0.045729, 0.078977, 0.366143, -0.088501, 0.037861, 0.241861, 0.106635, -0.014638, -0.028687, 0.191302, 0.059231, 0.100741, -0.025540);
            sum += i4 * mat4(0.033488, 0.041100, 0.153267, -0.101546, 0.085801, -0.326716, 0.043195, -0.238042, -0.016907, 0.079142, -0.018357, -0.237498, -0.043541, 0.003583, -0.036418, -0.011695);
            sum += i5 * mat4(-0.106284, 0.018137, 0.070130, 0.022089, -0.000509, -0.037182, 0.010259, -0.039426, 0.061949, 0.078499, -0.029198, -0.081410, 0.033228, -0.050886, -0.140174, -0.043200);
            sum += i6 * mat4(-0.166852, 0.109248, 0.149217, 0.206569, -0.066459, -0.191906, -0.083272, -0.023920, 0.035463, 0.096783, -0.018079, 0.029135, -0.069788, 0.107307, 0.039750, 0.018692);
            sum += i7 * mat4(-0.108150, 0.074894, -0.141851, -0.091946, -0.136710, 0.353538, -0.050274, 0.042484, -0.010035, 0.045315, -0.086755, 0.087829, 0.087608, -0.107601, -0.140381, 0.012779);
            sum += i8 * mat4(-0.251152, 0.031612, 0.056394, 0.058022, 0.025182, -0.146399, -0.069200, -0.032270, -0.023151, 0.215974, 0.036021, 0.019368, 0.033764, 0.106410, 0.045320, 0.026731);
        
            i0 = fetch(images[2], x0, y0);
            i1 = fetch(images[2], x1, y0);
            i2 = fetch(images[2], x2, y0);
            i3 = fetch(images[2], x0, y1);
            i4 = fetch(images[2], x1, y1);
            i5 = fetch(images[2], x2, y1);
            i6 = fetch(images[2], x0, y2);
            i7 = fetch(images[2], x1, y2);
            i8 = fetch(images[2], x2, y2);
        
            sum += i0 * mat4(0.028101, -0.079992, -0.226148, 0.312146, 0.045834, -0.230879, -0.016314, -0.760868, -0.219247, -0.071094, -0.037531, -0.126184, -0.205131, 0.329287, 0.295719, 0.332356);
            sum += i1 * mat4(-0.207376, -0.007007, 0.028568, -0.228654, -0.041620, 0.407648, -0.294113, 0.720390, -0.018366, 0.023874, -0.202797, 0.309073, 0.103421, -0.452698, -0.217830, -0.581984);
            sum += i2 * mat4(-0.071487, 0.100256, 0.084414, -0.199029, -0.275342, -0.184194, -0.151196, -0.577422, -0.055850, 0.201926, -0.059996, -0.347141, 0.056760, 0.295842, -0.004094, 0.164956);
            sum += i3 * mat4(0.052207, -0.113058, 0.055225, 0.060113, 0.284467, -0.000249, 0.188273, -0.609225, 0.417376, -0.103027, 0.270705, -0.403339, -0.155266, 0.008572, 0.277776, -0.011638);
            sum += i4 * mat4(0.078857, -0.225902, -0.033193, 0.067239, -0.796349, 0.190433, -0.341502, 0.478418, -0.155864, 0.359373, 0.107836, 0.467840, 0.578461, 0.119436, -0.124238, -0.092671);
            sum += i5 * mat4(-0.010180, 0.034175, -0.023057, -0.071963, -0.138120, -0.065195, 0.015737, -0.296888, -0.162929, 0.013603, -0.130346, 0.026926, -0.144254, -0.008787, -0.049469, 0.182777);
            sum += i6 * mat4(0.044678, -0.061006, -0.124459, 0.088193, 0.240406, -0.040035, -0.015667, -0.268800, 0.058966, -0.032536, 0.051835, 0.124543, -0.054630, 0.008140, -0.144701, 0.042710);
            sum += i7 * mat4(0.059259, -0.063317, 0.059809, -0.029448, -0.240953, -0.109429, -0.027658, -0.657104, -0.277464, -0.073638, 0.131984, -0.140460, -0.062391, -0.049692, 0.225866, 0.172917);
            sum += i8 * mat4(-0.087231, -0.213014, -0.099313, 0.041909, -0.334857, -0.032156, 0.058262, -0.063102, 0.171941, 0.018490, 0.108071, -0.113309, 0.187495, 0.069315, -0.042422, -0.126943);
        
        gl_FragColor = sum;
    
        }
    )

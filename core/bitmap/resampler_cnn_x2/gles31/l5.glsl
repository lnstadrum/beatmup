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
    uniform highp uint inputStride;
    uniform highp vec2 d1;
    layout(binding = 0, std430) readonly buffer inFeatures_ { highp uint data[][8]; } inFeatures;
    layout(binding = 1, rgba8) uniform writeonly lowp image2D outputImage;
    mediump vec2 getChroma(highp vec2 coord) {
        lowp vec3 i = texture(image, coord).rgb;
        return vec2(
            -0.168736 * i.r - 0.331264 * i.g + 0.500000 * i.b,
            +0.500000 * i.r - 0.418688 * i.g - 0.081312 * i.b
        );
    }
    void store(uvec2 dpos, mediump float y, mediump vec2 cc) {
        imageStore(outputImage, ivec2(2u * gl_GlobalInvocationID.xy + dpos),
            vec4(
                y + 1.402 * cc.y,
                y - 0.344136 * cc.x - 0.714136 * cc.y,
                y + 1.772 * cc.x,
                1.0
            )
        );
    }
    void main() {
        lowp vec4 f[4];
        for (int i = 0; i < 4; ++i)
            f[i] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][i]);
        mediump vec4 yy = vec4(0.066900, 0.040832, -0.030687, 0.02212) + vec4(
            dot(vec4(-0.128480,-0.331011,0.479560,-0.010360), f[0]) + dot(vec4(-0.288763,-0.163952,-0.107318,0.416246), f[1]) + dot(vec4(-0.111557,0.112956,-0.521888,0.357350), f[2]) + dot(vec4(0.530606,0.321276,-0.207054,0.483943), f[3]),
            dot(vec4(-0.164898,-0.142339,0.054280,0.323241), f[0]) + dot(vec4(0.457874,-0.204089,-0.129695,0.571394), f[1]) + dot(vec4(0.215299,-0.149313,-0.000150,-0.533151), f[2]) + dot(vec4(-0.348615,0.161753,-0.035195,0.245302), f[3]),
            dot(vec4(0.471591,0.245929,-0.160316,-0.030097), f[0]) + dot(vec4(-0.238465,0.593387,-0.079710,0.033316), f[1]) + dot(vec4(0.015486,0.092310,-0.267969,0.129855), f[2]) + dot(vec4(-0.013349,0.616188,0.280437,0.119231), f[3]),
            dot(vec4(0.104719,0.340025,-0.189702,0.320893), f[0]) + dot(vec4(-0.259299,-0.156502,0.393652,-0.257146), f[1]) + dot(vec4(0.036859,-0.127569,0.089679,-0.169039), f[2]) + dot(vec4(0.263064,0.270407,-0.136138,0.580510), f[3])
        );
        highp vec2 pos = vec2(gl_GlobalInvocationID.xy) * d1;
        store(uvec2(0u, 0u), yy[0], getChroma(pos));
        store(uvec2(1u, 0u), yy[1], getChroma(pos + vec2(d1.x,  0.0)));
        store(uvec2(0u, 1u), yy[2], getChroma(pos + vec2( 0.0, d1.y)));
        store(uvec2(1u, 1u), yy[3], getChroma(pos + vec2(d1.x, d1.y)));
    }
)

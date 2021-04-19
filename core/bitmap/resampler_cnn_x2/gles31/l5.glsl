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
    uniform lowp beatmupSampler image;
    uniform highp uint inputStride;
    uniform highp vec2 d1;
    layout(binding = 0, std430) readonly buffer inFeatures_ { highp uint data[][4]; } inFeatures;
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
        vec4 f[4];
        f[0] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][0]);
        f[1] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][1]);
        f[2] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][2]);
        f[3] = unpackUnorm4x8(inFeatures.data[ gl_GlobalInvocationID.y * inputStride + gl_GlobalInvocationID.x ][3]);
        mediump vec4 yy = vec4(-0.135934, 0.132361, -0.108204, -0.188912);
        yy += f[0] * mat4(-0.032274,-0.100460,-0.150648,0.085123,0.022339,-0.057390,-0.127189,-0.240535,-0.029194,0.248787,0.137417,0.063995,0.022615,-0.072361,0.153195,0.097375);
        yy += f[1] * mat4(0.474509,-0.088669,0.250664,-0.059446,0.531545,-0.073772,-0.087993,-0.087505,0.415351,0.073366,-0.105634,-0.082547,0.435940,0.090223,-0.078383,0.246641);
        yy += f[2] * mat4(0.693723,-0.569872,0.598486,0.063010,0.241609,-0.553002,0.591149,0.051025,0.721645,-0.546273,0.592602,-0.057942,0.291913,-0.544568,0.585166,-0.071107);
        yy += f[3] * mat4(-0.067522,-0.072288,0.116091,0.115563,-0.100090,0.418828,0.108549,-0.105875,0.106737,-0.035251,-0.099145,0.113255,0.073239,0.460255,-0.110799,-0.102701);
        highp vec2 pos = vec2(gl_GlobalInvocationID.xy) * d1;
        store(uvec2(0u, 0u), yy[0], getChroma(pos));
        store(uvec2(1u, 0u), yy[1], getChroma(pos + vec2(d1.x,  0.0)));
        store(uvec2(0u, 1u), yy[2], getChroma(pos + vec2( 0.0, d1.y)));
        store(uvec2(1u, 1u), yy[3], getChroma(pos + vec2(d1.x, d1.y)));
    }
)

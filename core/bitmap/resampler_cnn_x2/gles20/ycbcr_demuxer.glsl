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
uniform beatmupSampler image;
uniform sampler2D convnetOutput;
varying highp vec2 texCoord;

void main() {
    lowp vec4 yy = beatmupTexture(convnetOutput, texCoord);
    highp vec2 pos = mod(gl_FragCoord.xy, 2.0);
    
    lowp float y;
    if (pos.y < 1.0)
        if (pos.x < 1.0)
            y = yy[0];
        else
            y = yy[1];
    else
        if (pos.x < 1.0)
            y = yy[2];
        else
            y = yy[3];

    mediump vec3 i = beatmupTexture(image, texCoord).rgb;
    mediump float cb = -0.168736 * i.r - 0.331264 * i.g + 0.500000 * i.b;
    mediump float cr = +0.500000 * i.r - 0.418688 * i.g - 0.081312 * i.b;

    gl_FragColor = vec4(
        y + 1.402 * cr,
        y - 0.344136 * cb - 0.714136 * cr,
        y + 1.772 * cb,
        1.0
    );
}
)

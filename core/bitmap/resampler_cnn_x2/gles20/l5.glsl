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
            
        mediump vec4 sum = vec4(-0.135934, 0.132361, -0.108204, -0.188912);
    
        sum += texture2D(images[0], texCoord) * mat4(-0.032274,-0.100460,-0.150648,0.085123,0.022339,-0.057390,-0.127189,-0.240535,-0.029194,0.248787,0.137417,0.063995,0.022615,-0.072361,0.153195,0.097375);
        sum += texture2D(images[1], texCoord) * mat4(0.474509,-0.088669,0.250664,-0.059446,0.531545,-0.073772,-0.087993,-0.087505,0.415351,0.073366,-0.105634,-0.082547,0.435940,0.090223,-0.078383,0.246641);
        sum += texture2D(images[2], texCoord) * mat4(0.693723,-0.569872,0.598486,0.063010,0.241609,-0.553002,0.591149,0.051025,0.721645,-0.546273,0.592602,-0.057942,0.291913,-0.544568,0.585166,-0.071107);
        sum += texture2D(images[3], texCoord) * mat4(-0.067522,-0.072288,0.116091,0.115563,-0.100090,0.418828,0.108549,-0.105875,0.106737,-0.035251,-0.099145,0.113255,0.073239,0.460255,-0.110799,-0.102701);
        gl_FragColor = sum;
    
        }
    )
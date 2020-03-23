STRINGIFY(
        uniform sampler2D image;
        varying highp vec2 texCoord;
        uniform highp vec2 d1;
        uniform highp vec2 d2;

        lowp float sample(float x, float y) {
           return dot(texture2D(image, vec2(x, y)).rgb, vec3(0.299, 0.587, 0.114));
        }

        void main() {
            
        highp float
            x0 = texCoord.x - d2.x,
            x1 = texCoord.x - d1.x,
            x2 = texCoord.x,
            x3 = texCoord.x + d1.x,
            x4 = texCoord.x + d2.x,

            y0 = texCoord.y - d2.y,
            y1 = texCoord.y - d1.y,
            y2 = texCoord.y,
            y3 = texCoord.y + d1.y,
            y4 = texCoord.y + d2.y;
    
        lowp vec4 i[6];
        i[0] = vec4(sample(x0, y0), sample(x1, y0), sample(x2, y0), sample(x3, y0));
        i[1] = vec4(sample(x4, y0), sample(x0, y1), sample(x1, y1), sample(x2, y1));
        i[2] = vec4(sample(x3, y1), sample(x4, y1), sample(x0, y2), sample(x1, y2));
        i[3] = vec4(sample(x2, y2), sample(x3, y2), sample(x4, y2), sample(x0, y3));
        i[4] = vec4(sample(x1, y3), sample(x2, y3), sample(x3, y3), sample(x4, y3));
        i[5] = vec4(sample(x0, y4), sample(x1, y4), sample(x2, y4), sample(x3, y4));
        lowp float i6 = sample(x4, y4);
    
        gl_FragColor = vec4(
            dot(i[0], vec4(-0.014817, 0.044074, -0.012895, -0.027442)) + dot(i[1], vec4(0.023463, -0.021519, -0.005953, -0.007177)) +
            dot(i[2], vec4(0.029097, -0.014791, 0.031926, 0.055630)) + dot(i[3], vec4(1.119020, -1.128798, -0.073427, -0.019099)) +
            dot(i[4], vec4(-0.003262, 0.022114, -0.047463, 0.042775)) + dot(i[5], vec4(0.006275, 0.008267, -0.018914, 0.022875)) + i6 * -0.020396,
            dot(i[0], vec4(-0.267804, -0.039850, 0.246035, 0.183179)) + dot(i[1], vec4(-0.065458, -0.137852, 0.337786, 0.346353)) +
            dot(i[2], vec4(-0.088322, -0.197778, 0.242224, 0.510883)) + dot(i[3], vec4(-0.085094, -0.474302, -0.122845, 0.172770)) +
            dot(i[4], vec4(-0.044931, -0.517268, -0.311414, 0.184770)) + dot(i[5], vec4(-0.026061, -0.206575, -0.164712, 0.172643)) + i6 * 0.320035,
            dot(i[0], vec4(-0.014688, -0.051224, 0.069736, -0.024285)) + dot(i[1], vec4(0.042257, -0.062964, 0.114803, -0.439809)) +
            dot(i[2], vec4(0.319291, -0.083359, -0.086102, 0.250237)) + dot(i[3], vec4(-0.810499, 0.748503, -0.174948, 0.025047)) +
            dot(i[4], vec4(0.074661, -0.130384, 0.257242, -0.028880)) + dot(i[5], vec4(-0.042938, -0.032877, -0.002988, -0.001244)) + i6 * 0.007067,
            dot(i[0], vec4(-0.004627, 0.015154, -0.004228, 0.051322)) + dot(i[1], vec4(-0.015025, -0.024975, 0.047642, -0.024950)) +
            dot(i[2], vec4(0.003970, 0.020618, 0.059169, 1.027883)) + dot(i[3], vec4(0.017984, -0.045501, -0.013335, 0.022807)) +
            dot(i[4], vec4(-1.025664, -0.057483, 0.035912, 0.013304)) + dot(i[5], vec4(-0.046421, -0.070127, 0.031316, 0.030833)) + i6 * -0.027288
        ) + vec4(-0.00363, -0.00225, -0.04980, -0.00812);
    
        }
    )
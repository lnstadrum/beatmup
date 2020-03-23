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
            dot(i[0], vec4(0.013329, 0.014696, -0.200681, -0.155244)) + dot(i[1], vec4(0.047170, 0.292070, -0.225869, 0.349008)) +
            dot(i[2], vec4(0.488571, 0.147774, 0.413888, 0.203228)) + dot(i[3], vec4(-0.539972, -0.302837, -0.249915, -0.542748)) +
            dot(i[4], vec4(-0.589225, 0.219905, 0.016131, -0.017965)) + dot(i[5], vec4(-0.024980, -0.014388, 0.147897, 0.077816)) + i6 * 0.142268,
            dot(i[0], vec4(-0.003868, 0.018908, -0.004773, 0.047075)) + dot(i[1], vec4(-0.017970, -0.012331, -0.007848, -0.023367)) +
            dot(i[2], vec4(-0.021725, -0.003875, 0.030712, -0.078668)) + dot(i[3], vec4(-1.178379, -0.028957, -0.020911, -0.020515)) +
            dot(i[4], vec4(0.097525, 1.164249, 0.014801, 0.013916)) + dot(i[5], vec4(0.017167, -0.026322, 0.028922, 0.012524)) + i6 * 0.015665,
            dot(i[0], vec4(-0.014857, 0.042194, 0.108571, -0.039561)) + dot(i[1], vec4(0.043405, -0.216088, 0.473560, 0.525232)) +
            dot(i[2], vec4(-0.288009, -0.034586, -0.189960, -0.349686)) + dot(i[3], vec4(0.137605, -0.109740, -0.012389, 0.069963)) +
            dot(i[4], vec4(-0.535493, -0.135757, 0.311286, -0.231966)) + dot(i[5], vec4(0.094470, -0.145486, -0.424379, 0.613129)) + i6 * -0.097349,
            dot(i[0], vec4(0.009417, 0.111896, -0.027719, -0.057315)) + dot(i[1], vec4(0.034000, 0.025242, 0.040184, 0.132176)) +
            dot(i[2], vec4(0.064748, -0.013198, 0.117154, 0.013342)) + dot(i[3], vec4(0.044474, -0.040998, -0.056172, 0.118357)) +
            dot(i[4], vec4(-1.801624, 0.310814, 0.263336, -0.045090)) + dot(i[5], vec4(-0.026645, 0.055332, 0.163180, 0.021851)) + i6 * 0.079279
        ) + vec4(-0.01357, 0.00245, -0.00609, -0.00074);
    
        }
    )
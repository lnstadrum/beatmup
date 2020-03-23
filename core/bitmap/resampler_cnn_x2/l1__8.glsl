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
            dot(i[0], vec4(-0.088214, 0.065992, -0.021593, -0.089394)) + dot(i[1], vec4(0.065827, 0.171690, 0.097675, 0.007497)) +
            dot(i[2], vec4(-0.180378, 0.148217, -0.089750, 0.212011)) + dot(i[3], vec4(-0.347773, -0.047273, -0.051459, -0.052900)) +
            dot(i[4], vec4(0.044376, 0.057012, -0.050609, -0.133272)) + dot(i[5], vec4(-0.012790, 0.030787, 0.009663, 0.037184)) + i6 * 0.055910,
            dot(i[0], vec4(0.020759, 0.014796, 0.011263, 0.071472)) + dot(i[1], vec4(-0.017026, 0.003538, 0.043764, 0.008493)) +
            dot(i[2], vec4(0.039657, 0.111545, 0.002736, 0.026060)) + dot(i[3], vec4(0.163876, 0.154446, 0.140548, -0.011516)) +
            dot(i[4], vec4(0.090442, 0.022007, 0.076249, 0.016922)) + dot(i[5], vec4(-0.038109, 0.006924, 0.005037, -0.024331)) + i6 * 0.009837,
            dot(i[0], vec4(-0.061987, 0.020959, 0.026216, 0.037970)) + dot(i[1], vec4(0.015248, 0.028774, -0.037071, -0.278234)) +
            dot(i[2], vec4(0.226353, 0.039546, 0.144671, -0.027530)) + dot(i[3], vec4(-0.993174, 0.900412, 0.006300, 0.012512)) +
            dot(i[4], vec4(-0.157439, -0.094764, 0.166226, 0.015640)) + dot(i[5], vec4(-0.035354, 0.048929, 0.041625, -0.052942)) + i6 * 0.005755,
            dot(i[0], vec4(0.045235, -0.003324, 0.192478, 0.044499)) + dot(i[1], vec4(-0.131580, -0.342232, 0.073951, -0.038048)) +
            dot(i[2], vec4(0.047178, 0.137423, 0.055570, 0.263975)) + dot(i[3], vec4(-0.051868, -0.074321, -0.018170, -0.087588)) +
            dot(i[4], vec4(0.174123, 0.126914, -0.069024, -0.030879)) + dot(i[5], vec4(0.034258, -0.120671, -0.071889, 0.162537)) + i6 * -0.030323
        ) + vec4(0.21746, -0.00039, 0.00809, -0.01639);
    
        }
    )
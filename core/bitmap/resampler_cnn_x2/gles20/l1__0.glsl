STRINGIFY(
        beatmupInputImage image;
        varying highp vec2 texCoord;
        uniform highp vec2 d1;
        uniform highp vec2 d2;

        lowp float fetch(highp float x, highp float y) {
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
        i[0] = vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0));
        i[1] = vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1));
        i[2] = vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2));
        i[3] = vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3));
        i[4] = vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3));
        i[5] = vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4));
        lowp float i6 = fetch(x4, y4);
    
        gl_FragColor = vec4(
            dot(i[0], vec4(-0.007339, -0.027192, -0.012996, -0.004452)) + dot(i[1], vec4(0.017774, -0.044730, 0.013090, -0.010660)) +
            dot(i[2], vec4(0.097824, 0.058689, 0.046019, 0.084512)) + dot(i[3], vec4(0.100388, 0.114597, -0.038193, -0.034149)) +
            dot(i[4], vec4(0.141969, 0.038543, 0.064966, 0.006757)) + dot(i[5], vec4(0.004664, 0.083585, 0.094376, -0.017274)) + i6 * -0.003797,
            dot(i[0], vec4(0.040126, 0.077933, 0.122098, 0.167862)) + dot(i[1], vec4(0.011442, -0.071238, -0.071402, 0.013402)) +
            dot(i[2], vec4(-0.103351, -0.090477, 0.031883, 0.009317)) + dot(i[3], vec4(0.192935, -0.152668, -0.083256, -0.049399)) +
            dot(i[4], vec4(-0.069978, -0.163514, 0.255577, 0.142046)) + dot(i[5], vec4(-0.014577, 0.026698, 0.020632, -0.002863)) + i6 * 0.084354,
            dot(i[0], vec4(-0.042103, -0.003977, 0.000556, 0.011873)) + dot(i[1], vec4(-0.012383, -0.004933, 0.025093, 0.027100)) +
            dot(i[2], vec4(-0.059012, 0.009280, -0.028772, -0.030703)) + dot(i[3], vec4(0.216531, 0.111559, 0.029778, 0.047226)) +
            dot(i[4], vec4(0.198815, -0.257827, -0.125532, -0.009651)) + dot(i[5], vec4(0.066755, 0.094004, -0.085523, -0.071851)) + i6 * -0.051555,
            dot(i[0], vec4(0.016044, 0.025727, 0.025727, 0.011458)) + dot(i[1], vec4(0.013209, -0.010743, -0.026865, -0.076764)) +
            dot(i[2], vec4(0.012196, 0.038264, 0.057933, -0.050383)) + dot(i[3], vec4(-0.571076, 0.061163, -0.019401, 0.001056)) +
            dot(i[4], vec4(-0.010374, 0.098967, 0.073480, -0.012730)) + dot(i[5], vec4(-0.015934, 0.012291, 0.007990, 0.015988)) + i6 * 0.006966
        ) + vec4(0.01811, 0.16480, 0.29264, 0.28989);
    
        }
    )
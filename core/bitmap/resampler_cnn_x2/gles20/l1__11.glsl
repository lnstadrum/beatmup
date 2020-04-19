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
            dot(i[0], vec4(-0.013491, 0.170159, 0.092716, -0.043399)) + dot(i[1], vec4(0.015131, -0.017173, -0.015574, 0.050629)) +
            dot(i[2], vec4(0.001527, -0.016321, 0.024681, -0.136536)) + dot(i[3], vec4(0.108914, -0.086363, 0.014568, -0.016491)) +
            dot(i[4], vec4(-0.084914, -0.015133, -0.014245, -0.022697)) + dot(i[5], vec4(-0.011583, 0.088873, 0.076906, -0.039482)) + i6 * 0.000950,
            dot(i[0], vec4(-0.001407, -0.006870, 0.005030, -0.005271)) + dot(i[1], vec4(0.004417, -0.005361, -0.036059, 0.013328)) +
            dot(i[2], vec4(0.013483, 0.007208, 0.062571, 0.383315)) + dot(i[3], vec4(0.134840, -0.003986, -0.006864, -0.009768)) +
            dot(i[4], vec4(-0.442245, -0.195667, -0.001533, 0.001577)) + dot(i[5], vec4(-0.058801, 0.110178, 0.029117, -0.008393)) + i6 * 0.009255,
            dot(i[0], vec4(0.062007, 0.056668, 0.010828, -0.038990)) + dot(i[1], vec4(0.048361, 0.047653, -0.136290, 0.101153)) +
            dot(i[2], vec4(0.016429, 0.009483, 0.014956, 0.066102)) + dot(i[3], vec4(-0.084132, -0.000427, -0.021690, 0.081981)) +
            dot(i[4], vec4(-0.031741, -0.109593, -0.084248, -0.049324)) + dot(i[5], vec4(-0.024479, 0.068118, 0.011664, 0.011274)) + i6 * 0.030342,
            dot(i[0], vec4(-0.008758, -0.046346, 0.129422, 0.014272)) + dot(i[1], vec4(-0.020759, 0.023085, 0.024195, -0.585597)) +
            dot(i[2], vec4(-0.039093, 0.004241, -0.037777, 0.017734)) + dot(i[3], vec4(0.477839, 0.028153, 0.000773, 0.027276)) +
            dot(i[4], vec4(-0.027592, -0.006351, -0.011533, 0.023597)) + dot(i[5], vec4(-0.000341, -0.006421, 0.014950, 0.006821)) + i6 * -0.010114
        ) + vec4(0.07387, 0.00019, 0.18559, 0.00265);
    
        }
    )
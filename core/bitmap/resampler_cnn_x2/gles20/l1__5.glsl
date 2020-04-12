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
            dot(i[0], vec4(0.003372, 0.010346, 0.028879, 0.034747)) + dot(i[1], vec4(-0.004416, -0.003343, 0.007248, 0.000499)) +
            dot(i[2], vec4(0.017354, 0.033079, -0.014688, -0.046087)) + dot(i[3], vec4(0.162855, -0.299483, 0.023320, 0.014509)) +
            dot(i[4], vec4(-0.015098, -0.028363, -0.077357, 0.010820)) + dot(i[5], vec4(0.012530, 0.027314, 0.000452, 0.010086)) + i6 * -0.025278,
            dot(i[0], vec4(-0.029331, 0.034217, -0.125014, 0.096503)) + dot(i[1], vec4(-0.031839, -0.002914, 0.177124, -0.020958)) +
            dot(i[2], vec4(-0.004201, -0.008034, 0.182640, -0.544136)) + dot(i[3], vec4(0.307472, -0.147239, 0.064835, -0.072655)) +
            dot(i[4], vec4(0.210857, -0.074265, -0.006898, -0.001311)) + dot(i[5], vec4(0.030556, -0.102122, 0.035711, 0.016169)) + i6 * -0.003070,
            dot(i[0], vec4(0.012905, -0.025901, -0.001965, 0.010682)) + dot(i[1], vec4(0.014611, -0.000957, -0.031894, 0.152840)) +
            dot(i[2], vec4(-0.150948, 0.023268, 0.004523, 0.031033)) + dot(i[3], vec4(-0.266132, 0.275870, -0.067227, -0.002966)) +
            dot(i[4], vec4(-0.039817, 0.365378, -0.407248, 0.131827)) + dot(i[5], vec4(0.020957, -0.037858, -0.120354, 0.190019)) + i6 * -0.074561,
            dot(i[0], vec4(-0.002436, -0.018006, 0.057703, -0.005609)) + dot(i[1], vec4(0.009690, 0.017941, 0.002500, 0.017120)) +
            dot(i[2], vec4(-0.014459, -0.000572, -0.027506, 0.094723)) + dot(i[3], vec4(-0.000513, -0.000270, -0.010989, -0.028416)) +
            dot(i[4], vec4(-0.032685, -0.003615, 0.142899, 0.007404)) + dot(i[5], vec4(-0.026491, 0.016416, -0.068032, 0.075233)) + i6 * -0.018761
        ) + vec4(0.30588, -0.02849, 0.01796, 0.07817);
    
        }
    )
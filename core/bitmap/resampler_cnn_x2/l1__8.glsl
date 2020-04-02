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
            dot(i[0], vec4(-0.006476, 0.061439, -0.011630, -0.025089)) + dot(i[1], vec4(0.022029, 0.020364, -0.119760, -0.003126)) +
            dot(i[2], vec4(-0.013954, -0.069994, 0.017136, 0.051112)) + dot(i[3], vec4(0.145649, 0.038079, 0.106886, -0.035549)) +
            dot(i[4], vec4(-0.083289, -0.060003, -0.137615, -0.016392)) + dot(i[5], vec4(0.030396, 0.007221, 0.053810, 0.037477)) + i6 * -0.001606,
            dot(i[0], vec4(-0.001957, 0.075767, -0.065385, 0.034551)) + dot(i[1], vec4(-0.001717, 0.059517, -0.231996, 0.248520)) +
            dot(i[2], vec4(-0.169142, 0.038593, -0.200955, 0.505108)) + dot(i[3], vec4(-0.567553, 0.401226, -0.132257, 0.054503)) +
            dot(i[4], vec4(-0.197895, 0.247080, -0.153552, 0.046775)) + dot(i[5], vec4(0.008055, 0.026255, -0.038680, -0.000053)) + i6 * -0.002997,
            dot(i[0], vec4(-0.020042, -0.006651, 0.014626, -0.000382)) + dot(i[1], vec4(-0.000701, 0.038211, -0.016709, -0.050615)) +
            dot(i[2], vec4(-0.029283, 0.015894, -0.018405, 0.041859)) + dot(i[3], vec4(0.247107, -0.014759, -0.013611, 0.000683)) +
            dot(i[4], vec4(-0.036023, -0.460936, 0.067275, -0.029568)) + dot(i[5], vec4(-0.010172, 0.034393, -0.015834, 0.015956)) + i6 * 0.046963,
            dot(i[0], vec4(0.000531, -0.000511, -0.020189, -0.001747)) + dot(i[1], vec4(-0.015274, 0.014128, -0.044218, 0.127617)) +
            dot(i[2], vec4(0.002069, 0.016693, -0.060662, 0.395772)) + dot(i[3], vec4(-0.531938, -0.033878, -0.003812, 0.010900)) +
            dot(i[4], vec4(0.419825, -0.311265, -0.110172, -0.038000)) + dot(i[5], vec4(-0.017840, -0.026455, -0.018336, 0.073299)) + i6 * -0.004485
        ) + vec4(0.02958, -0.00501, 0.21504, 0.00925);
    
        }
    )
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
            dot(i[0], vec4(0.000286, -0.020340, -0.088156, -0.028325)) + dot(i[1], vec4(-0.002033, -0.028656, -0.042944, 0.006688)) +
            dot(i[2], vec4(0.062314, -0.018787, -0.025937, 0.146729)) + dot(i[3], vec4(0.249795, 0.018330, -0.014874, -0.006028)) +
            dot(i[4], vec4(-0.030634, 0.232171, -0.001396, -0.021788)) + dot(i[5], vec4(0.013486, -0.019495, -0.016514, 0.020779)) + i6 * 0.004117,
            dot(i[0], vec4(-0.024399, 0.005390, 0.008931, 0.011598)) + dot(i[1], vec4(-0.003760, 0.037167, -0.055192, -0.050542)) +
            dot(i[2], vec4(-0.033125, -0.002580, -0.163076, 0.481940)) + dot(i[3], vec4(0.303323, -0.090643, 0.010160, -0.008076)) +
            dot(i[4], vec4(-0.268641, -0.060211, -0.021166, 0.032610)) + dot(i[5], vec4(-0.005083, 0.060598, -0.002306, 0.006322)) + i6 * -0.025891,
            dot(i[0], vec4(-0.004984, 0.010944, -0.138692, -0.025641)) + dot(i[1], vec4(0.018747, 0.018186, 0.170545, 0.070725)) +
            dot(i[2], vec4(0.081390, 0.037850, -0.058859, 0.012245)) + dot(i[3], vec4(-0.098423, -0.044501, 0.043815, -0.017058)) +
            dot(i[4], vec4(-0.058138, -0.010043, -0.019155, 0.019740)) + dot(i[5], vec4(0.002710, 0.005076, 0.082820, 0.063496)) + i6 * 0.008864,
            dot(i[0], vec4(-0.029624, -0.059764, -0.188765, -0.023177)) + dot(i[1], vec4(0.085882, 0.009628, 0.018906, 0.416336)) +
            dot(i[2], vec4(0.229502, -0.048422, 0.047694, 0.090110)) + dot(i[3], vec4(0.010909, -0.292020, -0.162063, -0.042027)) +
            dot(i[4], vec4(-0.067440, -0.183951, -0.016370, 0.090882)) + dot(i[5], vec4(0.036610, -0.045061, 0.067328, 0.059440)) + i6 * 0.024088
        ) + vec4(0.14852, 0.01942, 0.14018, 0.01164);
    
        }
    )
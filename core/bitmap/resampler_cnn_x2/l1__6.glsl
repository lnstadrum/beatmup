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
            dot(i[0], vec4(0.089558, -0.033214, -0.066761, 0.026502)) + dot(i[1], vec4(-0.009299, -0.142194, 0.221790, 0.000842)) +
            dot(i[2], vec4(-0.046175, 0.003441, -0.022802, -0.199130)) + dot(i[3], vec4(0.078142, 0.200379, -0.052208, -0.002889)) +
            dot(i[4], vec4(0.078504, -0.129430, -0.113166, 0.085443)) + dot(i[5], vec4(0.024961, -0.002610, 0.060551, 0.014722)) + i6 * -0.048694,
            dot(i[0], vec4(0.020345, 0.038529, -0.013259, -0.000521)) + dot(i[1], vec4(-0.020725, 0.046312, 0.079143, 0.048599)) +
            dot(i[2], vec4(-0.046837, -0.001110, 0.014924, 0.197559)) + dot(i[3], vec4(0.176219, 0.049865, -0.005066, -0.038889)) +
            dot(i[4], vec4(0.062636, 0.119937, -0.006587, 0.028054)) + dot(i[5], vec4(-0.009746, 0.011315, -0.033712, -0.013668)) + i6 * -0.028172,
            dot(i[0], vec4(0.010686, -0.010363, 0.000186, -0.013082)) + dot(i[1], vec4(0.020033, -0.101680, -0.039978, -0.143235)) +
            dot(i[2], vec4(-0.018775, 0.004329, 0.130781, -0.042737)) + dot(i[3], vec4(0.168502, 0.084383, -0.002199, -0.047643)) +
            dot(i[4], vec4(0.059003, 0.049164, -0.121864, 0.064037)) + dot(i[5], vec4(0.008833, -0.061199, 0.038414, 0.025934)) + i6 * -0.052063,
            dot(i[0], vec4(-0.008339, 0.023243, 0.020348, -0.020002)) + dot(i[1], vec4(0.017244, 0.011593, 0.001868, -0.020100)) +
            dot(i[2], vec4(0.002583, 0.018854, -0.005063, 0.031820)) + dot(i[3], vec4(-0.752573, 0.176385, 0.022014, -0.001024)) +
            dot(i[4], vec4(0.038360, 0.146624, -0.038598, -0.019867)) + dot(i[5], vec4(0.010945, 0.021003, 0.042393, -0.007245)) + i6 * 0.016438
        ) + vec4(0.06648, 0.03128, 0.12839, 0.04037);
    
        }
    )
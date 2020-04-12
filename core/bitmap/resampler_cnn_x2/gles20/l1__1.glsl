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
            dot(i[0], vec4(-0.028639, -0.014469, 0.010502, -0.011672)) + dot(i[1], vec4(0.007274, 0.070278, 0.072763, -0.006793)) +
            dot(i[2], vec4(-0.051742, -0.007822, -0.174013, -0.283060)) + dot(i[3], vec4(0.165379, 0.083967, 0.094376, 0.089755)) +
            dot(i[4], vec4(0.260706, 0.016304, -0.207947, -0.122649)) + dot(i[5], vec4(-0.024478, -0.054048, -0.055423, 0.138902)) + i6 * 0.068719,
            dot(i[0], vec4(0.027096, 0.044969, -0.057724, -0.087544)) + dot(i[1], vec4(-0.004932, -0.023012, 0.097530, 0.095462)) +
            dot(i[2], vec4(-0.042509, -0.031429, -0.077268, 0.003717)) + dot(i[3], vec4(0.134177, 0.035817, 0.018051, -0.039575)) +
            dot(i[4], vec4(-0.087178, 0.029292, -0.005369, 0.002589)) + dot(i[5], vec4(0.017853, -0.043740, -0.005684, 0.001963)) + i6 * -0.007586,
            dot(i[0], vec4(0.048077, 0.077699, 0.056999, 0.044313)) + dot(i[1], vec4(-0.011781, 0.019242, 0.101020, -0.085398)) +
            dot(i[2], vec4(0.078876, -0.020268, -0.004729, 0.135453)) + dot(i[3], vec4(0.035604, -0.063434, 0.018678, 0.046564)) +
            dot(i[4], vec4(-0.069317, -0.239509, -0.155873, -0.028735)) + dot(i[5], vec4(-0.015418, 0.037840, 0.030653, -0.002150)) + i6 * 0.014256,
            dot(i[0], vec4(0.016845, 0.021509, -0.002490, -0.032850)) + dot(i[1], vec4(-0.006134, 0.106531, 0.087121, 0.071557)) +
            dot(i[2], vec4(0.001830, -0.059512, 0.061025, 0.044285)) + dot(i[3], vec4(0.093994, -0.018295, 0.039393, 0.002083)) +
            dot(i[4], vec4(0.074902, 0.050581, 0.038008, 0.002924)) + dot(i[5], vec4(-0.055179, -0.024380, 0.059818, 0.083638)) + i6 * -0.003429
        ) + vec4(0.01952, -0.00091, 0.31640, 0.03280);
    
        }
    )
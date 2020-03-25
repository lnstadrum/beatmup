STRINGIFY(
        uniform sampler2D image;
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
            dot(i[0], vec4(0.000253, 0.076685, -0.036285, -0.040615)) + dot(i[1], vec4(0.002380, -0.025732, 0.473854, 0.022404)) +
            dot(i[2], vec4(0.073259, -0.002254, -0.041898, -0.129028)) + dot(i[3], vec4(0.016708, 0.042232, 0.006211, 0.072084)) +
            dot(i[4], vec4(-0.048387, -0.001141, -0.006233, -0.027638)) + dot(i[5], vec4(-0.028117, -0.046715, -0.058194, 0.003426)) + i6 * 0.029525,
            dot(i[0], vec4(-0.001491, -0.001509, -0.040629, -0.043709)) + dot(i[1], vec4(-0.005187, 0.045842, 0.056788, -0.047440)) +
            dot(i[2], vec4(0.048167, 0.011486, 0.011142, -0.068941)) + dot(i[3], vec4(-0.046275, 0.013158, 0.008464, 0.035619)) +
            dot(i[4], vec4(0.033154, 0.704703, 0.154415, 0.013994)) + dot(i[5], vec4(-0.032078, -0.098843, -0.261154, -0.110366)) + i6 * -0.011855,
            dot(i[0], vec4(0.001747, 0.013693, 0.003709, 0.035539)) + dot(i[1], vec4(0.047902, 0.003306, 0.013490, 0.273158)) +
            dot(i[2], vec4(0.103922, 0.158415, 0.015082, 0.224855)) + dot(i[3], vec4(-0.036517, -1.376474, -0.094770, -0.000677)) +
            dot(i[4], vec4(0.023487, 0.169257, -0.047885, 0.061237)) + dot(i[5], vec4(0.012779, -0.035346, 0.110259, 0.017869)) + i6 * 0.102034,
            dot(i[0], vec4(-0.005269, -0.016573, -0.035020, 0.002555)) + dot(i[1], vec4(0.000962, -0.049169, -1.309081, -0.004947)) +
            dot(i[2], vec4(0.036979, -0.002907, -0.023616, 0.068008)) + dot(i[3], vec4(1.247375, 0.056408, -0.047526, -0.002299)) +
            dot(i[4], vec4(0.011631, 0.062513, 0.005285, 0.000274)) + dot(i[5], vec4(0.010108, 0.016616, -0.013687, -0.035495)) + i6 * 0.017694
        ) + vec4(0.09344, -0.02323, 0.12399, -0.00447);
    
        }
    )
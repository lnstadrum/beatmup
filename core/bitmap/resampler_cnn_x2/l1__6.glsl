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
            dot(i[0], vec4(0.192049, -0.149645, -0.013423, 0.031024)) + dot(i[1], vec4(-0.044787, -0.476730, 0.242059, 0.096918)) +
            dot(i[2], vec4(-0.079853, 0.057981, 0.725679, -0.457699)) + dot(i[3], vec4(-0.047005, 0.012998, 0.035292, -0.532930)) +
            dot(i[4], vec4(0.344316, -0.016814, -0.029037, 0.068420)) + dot(i[5], vec4(0.149798, -0.039664, -0.078108, 0.074680)) + i6 * -0.079012,
            dot(i[0], vec4(-0.017593, 0.020289, 0.193234, 0.200919)) + dot(i[1], vec4(-0.021796, 0.052517, -0.287806, -0.890259)) +
            dot(i[2], vec4(-0.797884, -0.057909, -0.077361, 0.242538)) + dot(i[3], vec4(0.666016, 0.567714, 0.084055, 0.074750)) +
            dot(i[4], vec4(0.188784, 0.056337, -0.029233, 0.019477)) + dot(i[5], vec4(-0.016420, -0.104509, -0.058121, -0.021317)) + i6 * -0.011600,
            dot(i[0], vec4(-0.031501, 0.026239, 0.010069, -0.084408)) + dot(i[1], vec4(0.013409, 0.082168, -0.181538, 0.521994)) +
            dot(i[2], vec4(-0.205961, 0.021438, 0.054574, -0.317351)) + dot(i[3], vec4(0.643845, -0.066393, -0.035032, -0.072414)) +
            dot(i[4], vec4(0.075823, -0.224880, -0.068028, 0.006081)) + dot(i[5], vec4(-0.002240, -0.036150, 0.042140, 0.009859)) + i6 * -0.002304,
            dot(i[0], vec4(-0.045589, 0.041785, 0.052780, 0.010341)) + dot(i[1], vec4(-0.074610, 0.069700, 0.126694, 0.161616)) +
            dot(i[2], vec4(0.007826, 0.052892, -0.039437, 0.136300)) + dot(i[3], vec4(0.169735, 0.040476, -0.057843, 0.034580)) +
            dot(i[4], vec4(-0.019929, -0.002641, 0.006619, 0.011921)) + dot(i[5], vec4(0.007604, 0.024021, 0.012267, -0.013702)) + i6 * -0.028908
        ) + vec4(-0.00301, 0.00278, -0.01148, -0.00006);
    
        }
    )
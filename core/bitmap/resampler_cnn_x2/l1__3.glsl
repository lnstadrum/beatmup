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
            dot(i[0], vec4(0.078203, -0.131817, -0.032340, 0.074973)) + dot(i[1], vec4(0.009515, 0.092379, 0.004499, -0.287522)) +
            dot(i[2], vec4(0.189141, -0.034354, -0.074955, 0.322328)) + dot(i[3], vec4(-0.591796, 0.133268, 0.166970, -0.081245)) +
            dot(i[4], vec4(0.296786, 0.210241, -0.498865, 0.086588)) + dot(i[5], vec4(-0.002232, -0.030915, 0.137076, -0.090473)) + i6 * -0.029351,
            dot(i[0], vec4(0.010707, 0.019313, 0.003919, -0.011498)) + dot(i[1], vec4(-0.004474, 0.020209, 0.012142, -0.043453)) +
            dot(i[2], vec4(-0.032747, -0.001472, -0.047025, -0.002324)) + dot(i[3], vec4(0.055536, 0.063032, 0.008471, 0.013487)) +
            dot(i[4], vec4(-0.332194, 0.437608, 0.102863, -0.024576)) + dot(i[5], vec4(0.038065, 0.095572, -0.064622, -0.055856)) + i6 * 0.005144,
            dot(i[0], vec4(-0.025587, -0.159749, 0.078351, 0.098416)) + dot(i[1], vec4(-0.052445, 0.048122, 0.328149, -0.203865)) +
            dot(i[2], vec4(-0.174307, 0.039032, -0.129206, -0.390487)) + dot(i[3], vec4(0.170706, 0.305576, -0.017967, 0.192480)) +
            dot(i[4], vec4(0.231857, -0.005512, -0.311401, 0.075351)) + dot(i[5], vec4(-0.112641, -0.081765, -0.036459, 0.156191)) + i6 * -0.045942,
            dot(i[0], vec4(0.001723, 0.012305, -0.006144, -0.001441)) + dot(i[1], vec4(0.003673, -0.010170, -0.051360, -0.003421)) +
            dot(i[2], vec4(-0.032208, 0.048357, 0.000074, -0.202867)) + dot(i[3], vec4(0.135975, 0.019874, -0.041481, -0.085605)) +
            dot(i[4], vec4(0.149819, 1.020644, -0.188432, -0.031534)) + dot(i[5], vec4(-0.000824, 0.126523, -0.045539, 0.057584)) + i6 * 0.012008
        ) + vec4(-0.03046, 0.18690, 0.03925, -0.69606);
    
        }
    )
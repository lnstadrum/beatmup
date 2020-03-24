STRINGIFY(
        uniform sampler2D image;
        varying highp vec2 texCoord;
        uniform highp vec2 d1;
        uniform highp vec2 d2;

        lowp float fetch(float x, float y) {
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
            dot(i[0], vec4(0.090538, 0.209921, -0.283581, -0.225151)) + dot(i[1], vec4(0.195807, 0.105406, -0.113482, -1.036427)) +
            dot(i[2], vec4(0.671539, -0.034562, -0.075012, -0.414841)) + dot(i[3], vec4(0.391524, 0.440114, -0.114883, -0.031292)) +
            dot(i[4], vec4(0.050680, 0.160376, -0.205851, -0.096198)) + dot(i[5], vec4(-0.002815, -0.020770, 0.033566, -0.043372)) + i6 * 0.182059,
            dot(i[0], vec4(0.075927, -0.098526, -0.145511, 0.253006)) + dot(i[1], vec4(-0.147194, 0.065542, 0.025236, -0.653937)) +
            dot(i[2], vec4(0.399938, 0.189758, 0.026203, 0.781545)) + dot(i[3], vec4(-0.265547, -0.640255, 0.236380, -0.331585)) +
            dot(i[4], vec4(-0.321188, 0.977554, -0.390806, -0.180164)) + dot(i[5], vec4(0.129498, -0.289778, 0.348600, -0.010468)) + i6 * -0.013378,
            dot(i[0], vec4(-0.019788, 0.054999, -0.007068, 0.116075)) + dot(i[1], vec4(-0.037899, 0.008401, 0.127393, 0.009339)) +
            dot(i[2], vec4(0.002987, 0.128968, -0.074462, 0.136791)) + dot(i[3], vec4(0.133600, 0.348180, 0.051912, 0.099329)) +
            dot(i[4], vec4(0.148259, 0.450278, -1.835480, -0.091979)) + dot(i[5], vec4(0.008070, -0.074283, 0.102625, 0.254598)) + i6 * 0.045608,
            dot(i[0], vec4(-0.015257, -0.017807, -0.103178, -0.003333)) + dot(i[1], vec4(-0.081017, -0.044319, 0.088196, 1.963119)) +
            dot(i[2], vec4(-0.163022, -0.102858, -0.102384, -0.101447)) + dot(i[3], vec4(-0.000120, -0.033905, -0.019238, 0.035792)) +
            dot(i[4], vec4(0.031724, -0.100235, 0.038031, 0.016483)) + dot(i[5], vec4(0.015118, -0.028216, 0.004835, -0.022806)) + i6 * -0.038369
        ) + vec4(-0.12170, -0.01878, -0.25766, -1.07325);
    
        }
    )
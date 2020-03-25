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
            dot(i[0], vec4(0.176329, -0.142675, -0.105468, 0.277892)) + dot(i[1], vec4(0.006965, -0.042697, 0.288158, 0.078819)) +
            dot(i[2], vec4(-0.364240, -0.218551, -0.017300, -0.419666)) + dot(i[3], vec4(0.186854, 0.393379, -0.178870, 0.063940)) +
            dot(i[4], vec4(0.420510, -0.699298, -0.958495, 0.818411)) + dot(i[5], vec4(-0.036953, -0.111635, 0.355458, 0.545129)) + i6 * -0.341632,
            dot(i[0], vec4(0.207514, -0.197800, 0.675986, -0.021042)) + dot(i[1], vec4(0.130234, -0.218082, -0.274843, -0.200924)) +
            dot(i[2], vec4(-0.240253, -0.230305, 0.068600, 0.135730)) + dot(i[3], vec4(-0.281797, -0.200985, 0.187269, 0.230256)) +
            dot(i[4], vec4(0.458202, 0.009672, 0.349610, 0.246821)) + dot(i[5], vec4(-0.230123, -0.047901, -0.698941, 0.492578)) + i6 * -0.341745,
            dot(i[0], vec4(-0.011830, 0.023336, 0.048706, 0.026886)) + dot(i[1], vec4(-0.005969, -0.013943, 0.125593, 1.684062)) +
            dot(i[2], vec4(0.098493, -0.036724, -0.000453, -0.130547)) + dot(i[3], vec4(-1.672621, -0.109207, 0.038690, 0.009969)) +
            dot(i[4], vec4(-0.007988, -0.071711, -0.007926, 0.013910)) + dot(i[5], vec4(0.020300, -0.018393, -0.011914, 0.011226)) + i6 * -0.003152,
            dot(i[0], vec4(-0.058961, 0.032271, -0.251225, 0.095995)) + dot(i[1], vec4(0.152841, 0.086596, -0.170848, 0.914566)) +
            dot(i[2], vec4(-0.548395, -0.057626, 0.011291, 0.150437)) + dot(i[3], vec4(0.292716, -0.395369, -0.104952, -0.001360)) +
            dot(i[4], vec4(-0.062788, -0.065153, 0.048017, 0.058857)) + dot(i[5], vec4(0.060983, -0.104852, 0.069058, 0.010864)) + i6 * -0.047271
        ) + vec4(-0.01752, -0.02873, -0.00146, 0.01711);
    
        }
    )
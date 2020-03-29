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
            dot(i[0], vec4(0.147637, -0.054329, -0.156704, 0.052154)) + dot(i[1], vec4(0.002719, 0.103989, -0.017805, -0.774651)) +
            dot(i[2], vec4(0.246230, 0.129309, -0.171075, -0.197084)) + dot(i[3], vec4(1.512053, -0.325447, -0.072878, -0.175839)) +
            dot(i[4], vec4(-0.287449, -0.548068, -0.278537, -0.425249)) + dot(i[5], vec4(0.150536, -0.176137, -0.462034, 0.541049)) + i6 * -0.029731,
            dot(i[0], vec4(-0.014190, 0.040824, 0.032109, -0.035298)) + dot(i[1], vec4(0.004647, 0.066069, 0.209163, 0.023134)) +
            dot(i[2], vec4(-0.037447, 0.003107, 0.028952, 0.172189)) + dot(i[3], vec4(0.219381, 0.050800, -0.033454, -0.032012)) +
            dot(i[4], vec4(0.103556, 0.117119, 0.005335, 0.045647)) + dot(i[5], vec4(-0.007743, 0.035790, -0.002169, -0.022026)) + i6 * -0.023176,
            dot(i[0], vec4(-0.018284, 0.063660, 0.020385, -0.042285)) + dot(i[1], vec4(0.007369, 0.002003, 0.221920, 0.042256)) +
            dot(i[2], vec4(0.027794, -0.005331, 0.086924, 0.358376)) + dot(i[3], vec4(0.228836, -0.024188, 0.090750, -0.027188)) +
            dot(i[4], vec4(-0.154022, -2.244780, 0.163604, 0.061693)) + dot(i[5], vec4(0.010472, -0.005991, 0.250610, 0.035100)) + i6 * -0.021615,
            dot(i[0], vec4(0.003079, -0.002654, -0.022807, -0.008995)) + dot(i[1], vec4(0.048871, -0.019538, -0.240374, -0.847190)) +
            dot(i[2], vec4(-0.028118, -0.013302, 0.010169, 0.136463)) + dot(i[3], vec4(1.107302, -0.080611, 0.034186, -0.041168)) +
            dot(i[4], vec4(0.099804, -0.159960, 0.091279, -0.031495)) + dot(i[5], vec4(0.054271, -0.047504, 0.063244, 0.008247)) + i6 * -0.031129
        ) + vec4(0.02630, 0.01914, 0.02791, 0.06441);
    
        }
    )
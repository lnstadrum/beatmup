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
            dot(i[0], vec4(-0.045745, 0.040226, 0.070341, -0.027701)) + dot(i[1], vec4(0.057605, 0.104666, -0.022482, -0.226305)) +
            dot(i[2], vec4(-0.086678, 0.030413, -0.019818, 0.136545)) + dot(i[3], vec4(-0.510071, -0.291873, 0.028188, -0.029751)) +
            dot(i[4], vec4(0.047693, 0.436065, 0.100104, 0.091870)) + dot(i[5], vec4(0.009202, -0.008632, -0.025997, -0.092727)) + i6 * 0.018972,
            dot(i[0], vec4(0.030294, -0.181718, 0.023562, 0.056131)) + dot(i[1], vec4(-0.037836, 0.028726, 0.356879, 0.913688)) +
            dot(i[2], vec4(0.023684, 0.047861, -0.036105, -0.229534)) + dot(i[3], vec4(-0.912763, -0.229521, 0.001950, -0.026720)) +
            dot(i[4], vec4(0.135810, 0.122154, -0.011128, -0.014617)) + dot(i[5], vec4(0.021248, -0.044985, -0.140935, 0.065785)) + i6 * 0.011707,
            dot(i[0], vec4(0.019628, -0.068914, -0.095054, 0.026167)) + dot(i[1], vec4(0.003157, -0.049473, 0.175721, 1.015119)) +
            dot(i[2], vec4(0.014188, 0.020830, 0.028555, -0.027622)) + dot(i[3], vec4(-1.313272, -0.002805, -0.012423, -0.010923)) +
            dot(i[4], vec4(0.019218, 0.123159, 0.049883, -0.060522)) + dot(i[5], vec4(0.027754, -0.063408, -0.139678, 0.037235)) + i6 * 0.028260,
            dot(i[0], vec4(-0.021713, 0.055731, 0.040160, -0.088863)) + dot(i[1], vec4(-0.111389, -0.043951, -0.021599, 0.116941)) +
            dot(i[2], vec4(-0.019000, 0.198895, -0.064902, -0.226618)) + dot(i[3], vec4(-0.026389, 0.161122, 0.065786, 0.039645)) +
            dot(i[4], vec4(-0.023418, 0.087401, -0.128320, 0.049446)) + dot(i[5], vec4(0.043799, 0.009163, 0.037965, -0.074504)) + i6 * -0.007970
        ) + vec4(0.20488, 0.03627, -0.00879, 0.09009);
    
        }
    )
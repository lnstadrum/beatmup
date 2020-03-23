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
            dot(i[0], vec4(-0.043052, 0.231841, -0.087136, -0.265126)) + dot(i[1], vec4(0.092621, -0.199916, 0.062560, -0.395856)) +
            dot(i[2], vec4(0.122662, 0.266380, 0.103362, 0.494072)) + dot(i[3], vec4(-0.048832, 0.369630, -0.144291, 0.412451)) +
            dot(i[4], vec4(-0.273187, -0.327747, -0.063535, -0.304731)) + dot(i[5], vec4(-0.174595, -0.232055, 0.196679, 0.124317)) + i6 * 0.087140,
            dot(i[0], vec4(0.036815, -0.280288, -0.127003, 0.178083)) + dot(i[1], vec4(0.066484, 0.340464, 0.067619, -0.589053)) +
            dot(i[2], vec4(-0.298798, -0.068509, 0.092860, 0.773734)) + dot(i[3], vec4(0.214601, -0.111236, -0.025376, -0.411488)) +
            dot(i[4], vec4(0.102149, 0.137027, -0.016953, 0.057073)) + dot(i[5], vec4(-0.107707, -0.012627, 0.037018, -0.066731)) + i6 * 0.005244,
            dot(i[0], vec4(-0.031809, -0.125220, 0.093178, -0.013962)) + dot(i[1], vec4(-0.017866, -0.008794, 0.353118, -0.844010)) +
            dot(i[2], vec4(0.648334, -0.106302, 0.055715, 0.090434)) + dot(i[3], vec4(-0.350560, 0.407537, -0.019274, -0.028250)) +
            dot(i[4], vec4(-0.085099, -0.020921, 0.093804, -0.067875)) + dot(i[5], vec4(0.006781, 0.003186, -0.057616, 0.056763)) + i6 * 0.008105,
            dot(i[0], vec4(0.097969, 0.064116, -0.001470, 0.079645)) + dot(i[1], vec4(-0.033774, -0.065405, 0.030398, 0.104958)) +
            dot(i[2], vec4(0.136960, 0.175340, 0.028842, 0.274521)) + dot(i[3], vec4(-1.769686, -0.263188, 0.101184, 0.116300)) +
            dot(i[4], vec4(0.183511, -0.148984, 0.079039, -0.168708)) + dot(i[5], vec4(0.049448, 0.017147, 0.015168, 0.142123)) + i6 * 0.036189
        ) + vec4(-0.00891, -0.00352, 0.03281, 0.05909);
    
        }
    )
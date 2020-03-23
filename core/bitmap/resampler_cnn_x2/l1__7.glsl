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
            dot(i[0], vec4(0.050448, 0.028293, -0.124135, 0.075050)) + dot(i[1], vec4(0.008563, 0.120619, -0.089280, -0.009567)) +
            dot(i[2], vec4(0.164329, -0.011403, -0.208764, 0.015964)) + dot(i[3], vec4(-0.716020, 0.218992, -0.116936, 0.122112)) +
            dot(i[4], vec4(-0.048695, 0.574137, 0.069079, 0.098778)) + dot(i[5], vec4(-0.051976, 0.102277, -0.274848, -0.089643)) + i6 * 0.065248,
            dot(i[0], vec4(-0.041216, -0.148198, 0.358675, 0.003282)) + dot(i[1], vec4(-0.207612, 0.146930, -0.365170, 0.615698)) +
            dot(i[2], vec4(-0.424491, 0.058114, -0.085078, 0.296063)) + dot(i[3], vec4(-0.433055, 0.037402, 0.114373, -0.165210)) +
            dot(i[4], vec4(0.619533, -0.706126, 0.287121, -0.028129)) + dot(i[5], vec4(0.134497, 0.127253, -0.295875, -0.002215)) + i6 * 0.071232,
            dot(i[0], vec4(-0.009200, 0.104277, -0.098836, 0.200948)) + dot(i[1], vec4(0.024715, -0.012998, -0.229669, 0.225290)) +
            dot(i[2], vec4(-0.680138, 0.058976, 0.075631, -0.141064)) + dot(i[3], vec4(-0.194837, -0.106407, 0.023605, -0.033600)) +
            dot(i[4], vec4(0.090551, 0.131798, 0.204025, 0.039220)) + dot(i[5], vec4(0.063023, -0.040469, 0.108202, -0.085008)) + i6 * 0.080357,
            dot(i[0], vec4(0.003978, 0.023932, -0.220910, 0.038464)) + dot(i[1], vec4(-0.012932, 0.054626, -0.008608, -0.202435)) +
            dot(i[2], vec4(0.022799, 0.128892, -0.080859, 0.322783)) + dot(i[3], vec4(0.265298, -0.255765, -0.200093, -0.028742)) +
            dot(i[4], vec4(0.089117, 0.198926, -0.086668, 0.107780)) + dot(i[5], vec4(0.060297, -0.108781, 0.076335, 0.055203)) + i6 * -0.004600
        ) + vec4(0.11033, -0.03471, 0.13192, -0.00760);
    
        }
    )
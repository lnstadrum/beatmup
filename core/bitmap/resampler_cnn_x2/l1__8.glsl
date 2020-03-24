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
            dot(i[0], vec4(-0.011087, 0.216449, -0.434609, 0.459637)) + dot(i[1], vec4(-0.246379, -0.048563, -0.237101, -0.567172)) +
            dot(i[2], vec4(0.263173, 0.594422, 0.048692, 0.030444)) + dot(i[3], vec4(0.980973, -0.615907, -0.462476, -0.062797)) +
            dot(i[4], vec4(-0.024043, -0.131838, 0.061088, 0.167038)) + dot(i[5], vec4(0.046583, -0.019012, 0.113360, -0.108295)) + i6 * -0.039055,
            dot(i[0], vec4(-0.054962, 0.031335, 0.094266, 0.017632)) + dot(i[1], vec4(-0.001485, -0.118900, -0.299160, 0.961943)) +
            dot(i[2], vec4(-0.436986, -0.085255, 0.265696, 0.268310)) + dot(i[3], vec4(-1.135567, 0.438983, 0.128231, -0.098859)) +
            dot(i[4], vec4(-0.011481, 0.007645, 0.052587, -0.020996)) + dot(i[5], vec4(0.048903, -0.003737, -0.047497, 0.030027)) + i6 * -0.010772,
            dot(i[0], vec4(-0.080778, 0.580075, 0.097603, -0.116078)) + dot(i[1], vec4(-0.001658, 0.437893, 0.645900, -0.234664)) +
            dot(i[2], vec4(-0.425649, -0.191143, 0.414285, 0.027117)) + dot(i[3], vec4(-0.915163, -0.423490, -0.032312, -0.298693)) +
            dot(i[4], vec4(-0.640757, -0.637142, 0.176945, 0.657057)) + dot(i[5], vec4(-0.036211, -0.262865, 0.214545, 0.730723)) + i6 * 0.330319,
            dot(i[0], vec4(-0.049208, 0.002362, -0.215272, 0.313199)) + dot(i[1], vec4(-0.057472, 0.213322, 0.528589, -0.026054)) +
            dot(i[2], vec4(-0.592058, -0.119871, -0.309988, -0.574731)) + dot(i[3], vec4(0.153187, 0.475193, 0.227435, 0.118527)) +
            dot(i[4], vec4(0.160083, -0.055744, -0.193262, 0.018037)) + dot(i[5], vec4(-0.016660, -0.062789, 0.123880, -0.074764)) + i6 * 0.001560
        ) + vec4(0.05142, -0.02635, -0.01518, 0.05243);
    
        }
    )
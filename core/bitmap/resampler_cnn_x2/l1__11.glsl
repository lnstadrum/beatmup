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
            dot(i[0], vec4(-0.059510, 0.000986, -0.014057, 0.140251)) + dot(i[1], vec4(-0.035004, -0.107302, -0.136200, 0.102562)) +
            dot(i[2], vec4(0.000433, 0.147345, 0.097080, -0.039710)) + dot(i[3], vec4(-0.293076, -0.019815, 0.011342, -0.020226)) +
            dot(i[4], vec4(0.368511, -0.166227, 0.010751, -0.123753)) + dot(i[5], vec4(-0.019475, -0.063632, 0.019659, 0.082000)) + i6 * 0.010416,
            dot(i[0], vec4(0.003803, -0.019054, 0.041658, -0.133065)) + dot(i[1], vec4(-0.088610, -0.058612, -0.068630, 0.374785)) +
            dot(i[2], vec4(0.410479, -0.152209, -0.011256, -0.059981)) + dot(i[3], vec4(0.026967, 0.028080, 0.027524, 0.007425)) +
            dot(i[4], vec4(0.003901, -0.015473, -0.067552, 0.008602)) + dot(i[5], vec4(-0.000956, 0.026701, -0.012309, -0.007177)) + i6 * 0.014440,
            dot(i[0], vec4(0.040634, -0.011473, -0.018462, -0.023923)) + dot(i[1], vec4(0.002205, -0.026647, -0.023438, 0.005045)) +
            dot(i[2], vec4(0.006955, 0.029622, 0.096340, -0.541889)) + dot(i[3], vec4(0.555511, -0.150172, 0.011672, 0.039937)) +
            dot(i[4], vec4(-0.339471, 0.446923, -0.115537, 0.009679)) + dot(i[5], vec4(-0.022664, -0.006589, 0.037256, 0.019440)) + i6 * -0.016014,
            dot(i[0], vec4(0.027527, -0.128564, 0.022754, 0.073962)) + dot(i[1], vec4(0.027449, 0.030516, -0.239883, -0.781841)) +
            dot(i[2], vec4(0.090160, -0.014835, 0.030182, 0.390897)) + dot(i[3], vec4(0.457737, -0.125710, 0.046371, -0.008343)) +
            dot(i[4], vec4(-0.077175, -0.018081, -0.012330, -0.015802)) + dot(i[5], vec4(0.007969, 0.013965, -0.042221, 0.011180)) + i6 * -0.007281
        ) + vec4(0.14054, -0.01750, 0.06037, -0.01038);
    
        }
    )
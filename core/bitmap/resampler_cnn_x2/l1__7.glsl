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
            dot(i[0], vec4(-0.059999, -0.260250, 0.091446, 0.758896)) + dot(i[1], vec4(0.054419, -0.049235, -0.362933, -0.510193)) +
            dot(i[2], vec4(0.521745, 0.364844, 0.180414, -0.118981)) + dot(i[3], vec4(-0.966977, -0.129929, 0.226128, 0.374942)) +
            dot(i[4], vec4(0.411778, -0.568452, -0.247238, -0.063816)) + dot(i[5], vec4(0.026051, 0.495729, 0.047390, -0.159057)) + i6 * -0.048894,
            dot(i[0], vec4(0.087852, 0.010880, -0.151602, 0.031994)) + dot(i[1], vec4(0.368001, -0.060257, -0.048257, -0.216747)) +
            dot(i[2], vec4(-0.603517, 0.267419, 0.041734, 0.059205)) + dot(i[3], vec4(0.375628, -0.631060, -0.471594, 0.123274)) +
            dot(i[4], vec4(0.184556, 0.400443, 0.282428, -0.239870)) + dot(i[5], vec4(-0.085600, -0.077696, 0.027490, 0.164950)) + i6 * 0.115391,
            dot(i[0], vec4(0.022008, 0.069189, 0.105162, -0.187708)) + dot(i[1], vec4(-0.010320, 0.028217, -0.216737, 0.284419)) +
            dot(i[2], vec4(0.048470, 0.154977, -0.053472, 0.013261)) + dot(i[3], vec4(-0.578241, 0.459882, 0.114412, -0.025711)) +
            dot(i[4], vec4(-0.033696, -0.124853, -0.182038, -0.001458)) + dot(i[5], vec4(0.019774, -0.037121, 0.101635, 0.011147)) + i6 * -0.004909,
            dot(i[0], vec4(0.119463, 0.233746, -0.189332, -0.132252)) + dot(i[1], vec4(0.022224, -0.022285, -0.108224, -0.246293)) +
            dot(i[2], vec4(0.209633, 0.011620, -0.153825, -0.236986)) + dot(i[3], vec4(-0.148835, 0.694832, -0.233609, -0.043128)) +
            dot(i[4], vec4(-0.358827, -0.153059, 0.275979, 0.384617)) + dot(i[5], vec4(0.144519, 0.117458, -0.280054, -0.342837)) + i6 * 0.387106
        ) + vec4(-0.01068, 0.01240, 0.13089, 0.04876);
    
        }
    )
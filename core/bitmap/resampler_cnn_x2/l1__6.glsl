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
            dot(i[0], vec4(-0.145237, -0.458049, 0.036716, 0.321952)) + dot(i[1], vec4(0.052832, 0.070654, 0.191009, 1.044108)) +
            dot(i[2], vec4(0.326713, -0.242152, 0.023712, 0.111198)) + dot(i[3], vec4(0.038040, -0.980111, -0.419724, -0.039073)) +
            dot(i[4], vec4(0.018823, -0.155666, -0.342717, 0.427040)) + dot(i[5], vec4(-0.032567, 0.086431, 0.030862, -0.137132)) + i6 * 0.155517,
            dot(i[0], vec4(0.055901, 0.096688, 0.407821, -0.119449)) + dot(i[1], vec4(-0.110015, -0.007431, -0.001441, 0.696668)) +
            dot(i[2], vec4(-0.810954, -0.055307, -0.052879, 0.066988)) + dot(i[3], vec4(0.262253, -0.517691, -0.104457, -0.059011)) +
            dot(i[4], vec4(0.201566, 0.114741, -0.067413, -0.020514)) + dot(i[5], vec4(0.048620, 0.034263, -0.038877, -0.056574)) + i6 * 0.026074,
            dot(i[0], vec4(0.002810, 0.095284, 0.165013, -0.116586)) + dot(i[1], vec4(0.169850, 0.160296, 0.082978, -0.211314)) +
            dot(i[2], vec4(0.304937, 0.111970, 0.021082, 0.105531)) + dot(i[3], vec4(0.210393, 0.020116, -0.098597, 0.004027)) +
            dot(i[4], vec4(0.088538, 0.030597, 0.108260, -0.121118)) + dot(i[5], vec4(-0.158874, -0.187873, -0.227861, -0.035263)) + i6 * -0.032201,
            dot(i[0], vec4(0.070311, 0.265293, 0.084957, -0.287698)) + dot(i[1], vec4(-0.225640, -0.228935, 0.166465, 0.688761)) +
            dot(i[2], vec4(0.481789, 0.195446, -0.300034, -0.703098)) + dot(i[3], vec4(-0.347000, -0.031975, 0.044730, 0.279704)) +
            dot(i[4], vec4(-0.008587, -0.118411, -0.140190, -0.089057)) + dot(i[5], vec4(0.039309, -0.009566, 0.065968, 0.106411)) + i6 * -0.024438
        ) + vec4(0.00105, 0.01415, 0.22578, 0.00160);
    
        }
    )
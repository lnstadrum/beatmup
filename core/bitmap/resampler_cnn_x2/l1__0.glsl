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
            dot(i[0], vec4(-0.113087, 0.159007, -0.006450, 0.132695)) + dot(i[1], vec4(-0.030826, -0.268347, 0.147049, 0.145138)) +
            dot(i[2], vec4(0.108871, -0.095132, -0.038116, -0.148436)) + dot(i[3], vec4(-0.215975, 0.070823, 0.138530, -0.094311)) +
            dot(i[4], vec4(0.067538, 0.202329, 0.160350, 0.100221)) + dot(i[5], vec4(0.202403, 0.115481, 0.102534, -0.149764)) + i6 * -0.192442,
            dot(i[0], vec4(0.035699, -0.037655, 0.027162, 0.130349)) + dot(i[1], vec4(-0.241498, 0.022102, -0.022084, 0.026166)) +
            dot(i[2], vec4(0.428938, -0.197020, -0.058344, -0.147453)) + dot(i[3], vec4(-0.159171, 0.567082, 0.207317, -0.038406)) +
            dot(i[4], vec4(-0.170806, -0.606515, -0.106993, 0.359257)) + dot(i[5], vec4(0.131589, 0.332509, -0.054885, -0.357730)) + i6 * -0.070488,
            dot(i[0], vec4(-0.002234, 0.038346, -0.064600, -0.048584)) + dot(i[1], vec4(0.067257, -0.002410, 0.052173, 0.127049)) +
            dot(i[2], vec4(0.257384, 0.123580, 0.010154, 0.134956)) + dot(i[3], vec4(-0.080336, -0.197908, 0.095092, 0.020048)) +
            dot(i[4], vec4(-0.027718, 0.130758, 0.126887, -0.108862)) + dot(i[5], vec4(0.229508, -0.020983, -0.212575, -0.293131)) + i6 * -0.082249,
            dot(i[0], vec4(-0.036029, 0.064163, -0.070282, -0.007205)) + dot(i[1], vec4(0.045391, 0.036238, 0.060686, 0.200165)) +
            dot(i[2], vec4(-0.127874, -0.088590, -0.079317, -0.018650)) + dot(i[3], vec4(-1.037166, 0.860700, 0.108251, 0.050010)) +
            dot(i[4], vec4(0.060715, 0.086595, -0.097876, -0.130256)) + dot(i[5], vec4(-0.029158, 0.031802, -0.032093, -0.019185)) + i6 * 0.095506
        ) + vec4(-0.01557, -0.00396, 0.05843, 0.09173);
    
        }
    )
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
            dot(i[0], vec4(-0.005415, 0.027937, -0.043226, -0.090663)) + dot(i[1], vec4(0.049970, 0.039955, 0.143024, 0.572372)) +
            dot(i[2], vec4(0.629123, 0.197881, 0.011922, -0.140609)) + dot(i[3], vec4(-0.604580, -0.658085, -0.191129, -0.051358)) +
            dot(i[4], vec4(-0.061473, 0.072615, 0.049405, -0.111935)) + dot(i[5], vec4(0.013769, -0.001551, 0.028326, 0.073018)) + i6 * 0.056744,
            dot(i[0], vec4(-0.010537, 0.027435, -0.003840, 0.024998)) + dot(i[1], vec4(0.031037, -0.051398, 0.070808, -0.044250)) +
            dot(i[2], vec4(-0.023438, -0.020554, 0.116078, 0.437734)) + dot(i[3], vec4(-0.720915, 0.081018, -0.013591, -0.111169)) +
            dot(i[4], vec4(0.225456, -0.095554, -0.004276, -0.022377)) + dot(i[5], vec4(-0.030567, 0.110023, -0.069602, 0.015836)) + i6 * -0.008639,
            dot(i[0], vec4(0.016306, 0.034277, -0.020202, 0.263705)) + dot(i[1], vec4(-0.055596, -0.015243, -0.081051, 0.073262)) +
            dot(i[2], vec4(-0.767085, 0.282048, -0.019710, 0.077374)) + dot(i[3], vec4(-0.082838, 0.515400, -0.179706, 0.038139)) +
            dot(i[4], vec4(-0.067409, 0.066657, -0.054139, -0.014401)) + dot(i[5], vec4(-0.015845, 0.029493, -0.008318, -0.031346)) + i6 * 0.019251,
            dot(i[0], vec4(-0.019701, 0.054100, 0.068543, -0.073321)) + dot(i[1], vec4(-0.072573, 0.215120, 0.372552, -0.161729)) +
            dot(i[2], vec4(-0.293936, -0.054389, -0.359952, -0.789009)) + dot(i[3], vec4(-0.068750, 0.771726, 0.368445, 0.062559)) +
            dot(i[4], vec4(0.351027, 0.323493, -0.445411, -0.259372)) + dot(i[5], vec4(0.084295, 0.029094, -0.128470, 0.000935)) + i6 * 0.009036
        ) + vec4(-0.00636, 0.12220, 0.03231, 0.00375);
    
        }
    )
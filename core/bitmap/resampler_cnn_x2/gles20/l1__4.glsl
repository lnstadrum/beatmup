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
            dot(i[0], vec4(-0.031332, 0.023129, 0.026615, -0.013575)) + dot(i[1], vec4(0.010876, 0.042693, -0.090434, 0.049042)) +
            dot(i[2], vec4(-0.036854, 0.000590, -0.026905, 0.085271)) + dot(i[3], vec4(-0.138757, 0.081052, -0.017047, -0.031294)) +
            dot(i[4], vec4(0.094652, -0.016481, -0.033619, 0.017333)) + dot(i[5], vec4(-0.026636, 0.018587, 0.138480, -0.091029)) + i6 * 0.020698,
            dot(i[0], vec4(-0.001023, 0.005518, -0.026120, -0.009958)) + dot(i[1], vec4(0.051175, -0.007292, 0.067963, -0.387802)) +
            dot(i[2], vec4(-0.310933, 0.127710, 0.019081, 0.053682)) + dot(i[3], vec4(0.101432, 0.007836, 0.053098, -0.004550)) +
            dot(i[4], vec4(-0.022768, -0.010034, 0.006572, -0.004304)) + dot(i[5], vec4(0.007775, -0.006954, -0.000800, 0.016453)) + i6 * 0.005620,
            dot(i[0], vec4(0.029678, -0.017024, 0.017719, -0.006615)) + dot(i[1], vec4(0.003802, -0.017049, 0.025229, 0.107532)) +
            dot(i[2], vec4(0.016498, 0.023218, -0.053261, 0.019536)) + dot(i[3], vec4(0.033108, 0.107397, -0.032505, 0.033544)) +
            dot(i[4], vec4(-0.087931, 0.053438, -0.102351, 0.026639)) + dot(i[5], vec4(-0.020453, 0.053025, 0.029657, -0.009550)) + i6 * -0.004077,
            dot(i[0], vec4(-0.007297, -0.008329, 0.010772, 0.002176)) + dot(i[1], vec4(-0.001902, 0.007485, -0.046187, 0.047106)) +
            dot(i[2], vec4(-0.008819, -0.012661, 0.007239, 0.112963)) + dot(i[3], vec4(-0.069185, -0.043063, 0.010606, -0.060626)) +
            dot(i[4], vec4(0.513711, -0.549151, 0.061552, 0.023771)) + dot(i[5], vec4(0.035981, 0.003373, -0.021666, -0.029625)) + i6 * 0.012625
        ) + vec4(0.01995, 0.22652, 0.04117, -0.00246);
    
        }
    )
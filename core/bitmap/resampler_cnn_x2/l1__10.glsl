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
            dot(i[0], vec4(-0.007557, -0.051532, 0.043154, 0.024060)) + dot(i[1], vec4(0.041129, -0.057844, -0.013525, 0.204634)) +
            dot(i[2], vec4(-0.117278, -0.125015, 0.080779, 0.162861)) + dot(i[3], vec4(0.623265, -0.901887, -0.263244, -0.057214)) +
            dot(i[4], vec4(-0.129670, -0.537705, 0.737159, 0.329141)) + dot(i[5], vec4(0.035475, 0.060712, -0.057471, -0.156644)) + i6 * 0.037105,
            dot(i[0], vec4(0.009478, -0.063355, 0.043523, 0.007773)) + dot(i[1], vec4(-0.017518, -0.002116, 0.095830, 0.188708)) +
            dot(i[2], vec4(0.133683, 0.008402, 0.049927, 0.137968)) + dot(i[3], vec4(0.220105, 0.124336, 0.048835, -0.049767)) +
            dot(i[4], vec4(0.070313, 0.037293, -0.015484, -0.018124)) + dot(i[5], vec4(-0.000762, -0.037572, 0.034796, -0.005701)) + i6 * -0.024323,
            dot(i[0], vec4(-0.030327, 0.051672, -0.030148, -0.083888)) + dot(i[1], vec4(-0.015721, -0.069098, 0.065547, -0.031212)) +
            dot(i[2], vec4(-0.093300, -0.023312, -0.088368, -0.144506)) + dot(i[3], vec4(1.379193, 0.287202, -0.086850, -0.048529)) +
            dot(i[4], vec4(-0.110770, -0.038329, -0.055821, 0.013490)) + dot(i[5], vec4(0.044752, 0.023864, -0.080218, -0.057885)) + i6 * -0.030156,
            dot(i[0], vec4(-0.005225, 0.064847, -0.005615, -0.054243)) + dot(i[1], vec4(0.009403, -0.099617, 0.046999, -0.025150)) +
            dot(i[2], vec4(-0.087525, 0.022606, 0.092565, -0.168385)) + dot(i[3], vec4(0.883390, -0.756564, 0.033559, 0.118661)) +
            dot(i[4], vec4(0.004111, 0.374562, -0.495831, 0.144764)) + dot(i[5], vec4(-0.076858, -0.001936, -0.000542, 0.066180)) + i6 * -0.109648
        ) + vec4(0.01494, 0.00310, -0.64429, -0.03286);
    
        }
    )
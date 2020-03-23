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
            dot(i[0], vec4(-0.049729, 0.056501, -0.020871, -0.001660)) + dot(i[1], vec4(-0.026170, -0.057246, 0.089485, 0.205036)) +
            dot(i[2], vec4(-0.102747, 0.022608, -0.067546, 0.386212)) + dot(i[3], vec4(-0.541897, -0.019958, -0.002170, -0.008047)) +
            dot(i[4], vec4(-0.030848, 0.042447, 0.033167, 0.000022)) + dot(i[5], vec4(-0.001061, 0.043237, -0.100040, 0.054094)) + i6 * -0.053048,
            dot(i[0], vec4(-0.006398, -0.100491, 0.094532, 0.059860)) + dot(i[1], vec4(0.027668, -0.073985, 0.023823, -0.103711)) +
            dot(i[2], vec4(0.151159, -0.057143, -0.011877, 0.174936)) + dot(i[3], vec4(-0.155143, 0.009584, -0.043975, 0.033053)) +
            dot(i[4], vec4(-0.042211, 0.038018, 0.076211, -0.021748)) + dot(i[5], vec4(0.012221, -0.008276, 0.001690, -0.024385)) + i6 * 0.003049,
            dot(i[0], vec4(-0.022124, 0.040713, -0.003719, -0.025940)) + dot(i[1], vec4(0.105996, -0.042446, 0.003011, 0.015263)) +
            dot(i[2], vec4(0.034210, 0.018714, 0.075278, 0.056662)) + dot(i[3], vec4(-1.343807, 0.183110, 0.044611, 0.049725)) +
            dot(i[4], vec4(-0.258317, -0.239096, 0.096451, 0.101488)) + dot(i[5], vec4(0.044362, 0.113254, 0.161605, 0.026621)) + i6 * -0.079269,
            dot(i[0], vec4(-0.041300, 0.040182, 0.061657, 0.168398)) + dot(i[1], vec4(-0.049563, -0.093729, 0.056119, -0.488143)) +
            dot(i[2], vec4(-0.072675, -0.084306, 0.069594, 0.163678)) + dot(i[3], vec4(0.429347, 0.127837, 0.009894, -0.005716)) +
            dot(i[4], vec4(-0.320139, 0.001074, -0.247364, 0.101350)) + dot(i[5], vec4(0.051536, 0.023383, 0.097398, -0.017662)) + i6 * 0.022097
        ) + vec4(0.12555, 0.09594, 0.02840, 0.03362);
    
        }
    )
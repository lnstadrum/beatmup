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
            dot(i[0], vec4(0.019312, -0.086370, 0.127180, -0.041334)) + dot(i[1], vec4(-0.007119, 0.001850, 0.029548, -0.163600)) +
            dot(i[2], vec4(-0.082413, -0.022147, -0.012618, -0.233757)) + dot(i[3], vec4(0.670906, 0.369567, 0.006493, -0.116014)) +
            dot(i[4], vec4(0.263757, 0.011455, 0.155410, -0.017892)) + dot(i[5], vec4(-0.021672, 0.097283, -0.075746, 0.038700)) + i6 * 0.001188,
            dot(i[0], vec4(-0.005275, -0.292715, 0.419153, -0.134627)) + dot(i[1], vec4(-0.014460, 0.052524, -0.589036, 1.468740)) +
            dot(i[2], vec4(-0.131902, -0.009686, 0.103448, -0.090758)) + dot(i[3], vec4(-0.556532, 0.008578, 0.025958, -0.033649)) +
            dot(i[4], vec4(0.024060, -0.121056, 0.041363, -0.067593)) + dot(i[5], vec4(-0.009258, 0.016706, -0.000253, 0.024549)) + i6 * -0.012444,
            dot(i[0], vec4(0.112384, -0.089107, 0.074748, -0.042983)) + dot(i[1], vec4(0.021501, 0.049454, -0.227357, 0.210724)) +
            dot(i[2], vec4(-0.018382, -0.072715, -0.042938, -0.323790)) + dot(i[3], vec4(0.601339, -0.533077, -0.030263, -0.320453)) +
            dot(i[4], vec4(0.959364, -0.378418, -0.080219, 0.234726)) + dot(i[5], vec4(0.130477, 0.340031, -1.102810, 0.687184)) + i6 * -0.146431,
            dot(i[0], vec4(0.060408, -0.033203, 0.217134, -0.075493)) + dot(i[1], vec4(-0.050232, 0.010816, 0.033628, -1.347945)) +
            dot(i[2], vec4(0.894574, 0.112724, -0.098295, -0.061945)) + dot(i[3], vec4(-0.087329, 0.126046, 0.074944, -0.001088)) +
            dot(i[4], vec4(0.043188, 0.092232, 0.043319, -0.049929)) + dot(i[5], vec4(0.009549, 0.018410, -0.037553, 0.008333)) + i6 * -0.007178
        ) + vec4(-0.00914, -0.06278, -0.01344, 0.17456);
    
        }
    )
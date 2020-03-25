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
            dot(i[0], vec4(-0.003584, -0.014194, -0.002981, -0.013444)) + dot(i[1], vec4(0.013589, 0.011459, 0.067722, 0.053087)) +
            dot(i[2], vec4(0.057184, -0.012971, 0.016269, 0.128637)) + dot(i[3], vec4(0.185534, 0.417907, 0.010011, 0.004010)) +
            dot(i[4], vec4(0.031789, 0.018557, 0.000564, -0.014413)) + dot(i[5], vec4(-0.003794, 0.007993, -0.019044, 0.004322)) + i6 * -0.006244,
            dot(i[0], vec4(-0.070725, -0.137062, -0.150777, -0.012922)) + dot(i[1], vec4(0.136323, 0.285244, -0.034543, 0.199028)) +
            dot(i[2], vec4(0.240547, -0.316896, -0.231523, 0.385186)) + dot(i[3], vec4(-0.632164, -0.128827, 0.067463, 0.485953)) +
            dot(i[4], vec4(-0.312158, -0.127685, 0.117954, 0.067682)) + dot(i[5], vec4(-0.329405, -0.082636, 0.022277, 0.037613)) + i6 * -0.091639,
            dot(i[0], vec4(0.019953, 0.085744, 0.127132, -0.185869)) + dot(i[1], vec4(-0.085708, -0.114621, -0.121849, 0.917681)) +
            dot(i[2], vec4(-0.772161, 0.122606, -0.111117, -0.022439)) + dot(i[3], vec4(0.766020, -0.661176, 0.032826, -0.073171)) +
            dot(i[4], vec4(0.062029, 0.036442, 0.033270, -0.092628)) + dot(i[5], vec4(0.075297, -0.033389, -0.064222, 0.071686)) + i6 * -0.013071,
            dot(i[0], vec4(0.000456, 0.010034, -0.004080, 0.007079)) + dot(i[1], vec4(-0.003220, -0.007031, -0.039615, -0.012529)) +
            dot(i[2], vec4(-0.011881, 0.016583, -0.007186, -0.054328)) + dot(i[3], vec4(0.050934, 0.522018, -0.031254, -0.007671)) +
            dot(i[4], vec4(-0.032329, -0.024630, -0.019806, -0.003946)) + dot(i[5], vec4(0.002164, -0.014302, -0.007492, -0.007473)) + i6 * -0.000634
        ) + vec4(0.02210, -0.02523, -0.00463, 0.18767);
    
        }
    )
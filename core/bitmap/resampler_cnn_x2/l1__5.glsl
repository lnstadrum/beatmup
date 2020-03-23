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
            dot(i[0], vec4(-0.001550, -0.036205, 0.059970, -0.061487)) + dot(i[1], vec4(0.029153, 0.009529, 0.032102, 0.118040)) +
            dot(i[2], vec4(-0.020515, -0.016779, 0.107734, -0.244869)) + dot(i[3], vec4(-0.134733, -0.020282, -0.030247, 0.013732)) +
            dot(i[4], vec4(0.174377, -0.003750, 0.125704, -0.023271)) + dot(i[5], vec4(0.031975, -0.061273, -0.009429, -0.020860)) + i6 * -0.012167,
            dot(i[0], vec4(0.002537, 0.053640, 0.084987, -0.022217)) + dot(i[1], vec4(0.044360, 0.013654, 0.109255, 0.346601)) +
            dot(i[2], vec4(0.266460, -0.061068, -0.027350, 0.000031)) + dot(i[3], vec4(0.149537, -0.042018, -0.016873, -0.014094)) +
            dot(i[4], vec4(-0.090260, 0.081609, -0.103565, 0.005034)) + dot(i[5], vec4(0.002361, 0.005968, 0.020326, -0.017087)) + i6 * 0.018372,
            dot(i[0], vec4(-0.115063, 0.277651, -0.323226, 0.391088)) + dot(i[1], vec4(-0.282676, 0.013688, -0.095714, 0.177856)) +
            dot(i[2], vec4(-0.269784, 0.313018, 0.076890, -0.011940)) + dot(i[3], vec4(0.045627, 0.138974, -0.153263, -0.267159)) +
            dot(i[4], vec4(0.255538, -0.320257, 0.176313, 0.023422)) + dot(i[5], vec4(0.075385, -0.097825, 0.057722, -0.069000)) + i6 * -0.035808,
            dot(i[0], vec4(0.088815, -0.184576, 0.195639, -0.215890)) + dot(i[1], vec4(0.108124, -0.100026, 0.229094, -0.290411)) +
            dot(i[2], vec4(0.309339, -0.151831, 0.090704, -0.202698)) + dot(i[3], vec4(0.285474, -0.302926, 0.144615, -0.178509)) +
            dot(i[4], vec4(0.341319, -0.410613, 0.454403, -0.224257)) + dot(i[5], vec4(0.103113, -0.191457, 0.241489, -0.278367)) + i6 * 0.139142
        ) + vec4(0.15882, 0.05366, -0.00250, 0.00175);
    
        }
    )
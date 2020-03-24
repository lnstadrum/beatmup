STRINGIFY(
        uniform sampler2D images[3];
        varying highp vec2 texCoord;
        uniform highp vec2 d1;

        lowp vec4 fetch(sampler2D image, float x, float y) {
            return texture2D(image, vec2(x, y));
        }

        void main() {
            
        highp float
            x0 = texCoord.x - d1.x,
            x1 = texCoord.x,
            x2 = texCoord.x + d1.x,

            y0 = texCoord.y - d1.y,
            y1 = texCoord.y,
            y2 = texCoord.y + d1.y;
    
        vec4 i0[3], i1[3], i2[3], i3[3], i4[3], i5[3], i6[3], i7[3], i8[3];
        for (int i = 0; i < 3; ++i) {
            i0[i] = fetch(images[i], x0, y0);
            i1[i] = fetch(images[i], x1, y0);
            i2[i] = fetch(images[i], x2, y0);
            i3[i] = fetch(images[i], x0, y1);
            i4[i] = fetch(images[i], x1, y1);
            i5[i] = fetch(images[i], x2, y1);
            i6[i] = fetch(images[i], x0, y2);
            i7[i] = fetch(images[i], x1, y2);
            i8[i] = fetch(images[i], x2, y2);
        }
    
        gl_FragColor = vec4(
            dot(vec4(0.009194, 0.190543, -0.137893, 0.145370), i0[0])
                + dot(vec4(0.007927, -0.022212, -0.251002, 0.093474), i1[0])
                + dot(vec4(0.129752, -0.050142, -0.130436, 0.188059), i2[0])
                + dot(vec4(0.081919, -0.289620, -0.031772, 0.143231), i3[0])
                + dot(vec4(0.209235, 0.219829, 0.077665, 0.270882), i4[0])
                + dot(vec4(0.110394, -0.053808, -0.139753, 0.113066), i5[0])
                + dot(vec4(0.041825, 0.097168, 0.248854, 0.251498), i6[0])
                + dot(vec4(-0.050388, 0.314488, 0.247064, 0.209160), i7[0])
                + dot(vec4(0.087320, 0.248866, 0.270689, 0.150501), i8[0])
                + dot(vec4(0.054073, 0.054799, -0.036834, -0.017627), i0[1])
                + dot(vec4(0.186503, 0.115824, -0.070632, 0.129387), i1[1])
                + dot(vec4(0.214276, 0.172829, 0.466865, -0.162800), i2[1])
                + dot(vec4(0.071195, 0.011962, 0.147329, 0.088029), i3[1])
                + dot(vec4(0.174222, 0.045158, -0.191182, -0.057973), i4[1])
                + dot(vec4(0.016396, -0.010700, -0.370714, 0.044593), i5[1])
                + dot(vec4(0.291143, 0.131097, 0.253177, 0.104369), i6[1])
                + dot(vec4(0.093475, 0.096119, -0.012187, 0.043436), i7[1])
                + dot(vec4(0.252016, 0.123782, -0.061759, -0.090116), i8[1])
                + dot(vec4(-0.048027, -0.362938, 0.041203, -0.042290), i0[2])
                + dot(vec4(-0.188990, 0.037523, -0.005980, -0.003101), i1[2])
                + dot(vec4(-0.351591, -0.156850, 0.111447, -0.011126), i2[2])
                + dot(vec4(-0.130996, -0.513930, 0.112340, -0.194108), i3[2])
                + dot(vec4(-0.054962, 0.211621, -0.058071, 0.028993), i4[2])
                + dot(vec4(-0.319879, 0.071724, 0.062777, -0.236649), i5[2])
                + dot(vec4(-0.074599, -0.574581, 0.189765, -0.165303), i6[2])
                + dot(vec4(-0.085179, -0.052491, 0.079441, 0.072774), i7[2])
                + dot(vec4(-0.265189, -0.108635, 0.076291, -0.225303), i8[2]),
            dot(vec4(0.041739, -0.160291, -0.126540, 0.228070), i0[0])
                + dot(vec4(-0.174930, 0.132773, -0.422519, 0.143413), i1[0])
                + dot(vec4(-0.112349, 0.122935, 0.245343, 0.566007), i2[0])
                + dot(vec4(0.276119, -0.335422, 0.549605, 0.418584), i3[0])
                + dot(vec4(0.233562, 0.453341, 0.141017, 0.148693), i4[0])
                + dot(vec4(0.027302, 0.176491, -0.210048, 0.144990), i5[0])
                + dot(vec4(-0.323361, -0.054255, -0.083865, -0.019008), i6[0])
                + dot(vec4(-0.029080, -0.752303, 0.193961, -0.237221), i7[0])
                + dot(vec4(-0.163897, 0.015481, -0.230999, 0.200799), i8[0])
                + dot(vec4(-0.157059, 0.340652, 0.164558, 0.069803), i0[1])
                + dot(vec4(0.018121, 0.163768, 0.236358, -0.057614), i1[1])
                + dot(vec4(0.204921, 0.252575, -0.413044, -0.043531), i2[1])
                + dot(vec4(0.011892, 0.141703, -0.813406, -0.131280), i3[1])
                + dot(vec4(0.139544, -0.158797, -0.444338, 0.115728), i4[1])
                + dot(vec4(0.002567, 0.101395, 0.670019, -0.056075), i5[1])
                + dot(vec4(0.055951, -0.069999, 0.292765, -0.256618), i6[1])
                + dot(vec4(0.020861, -0.008582, -0.010652, 0.074708), i7[1])
                + dot(vec4(0.128426, 0.109624, 0.322507, 0.063357), i8[1])
                + dot(vec4(-0.193767, 0.046303, -0.058692, 0.346672), i0[2])
                + dot(vec4(-0.188343, 0.033920, -0.019166, 0.252241), i1[2])
                + dot(vec4(0.037257, -0.100453, -0.098428, 0.017457), i2[2])
                + dot(vec4(0.096164, -0.043671, 0.030479, 0.015246), i3[2])
                + dot(vec4(-0.055953, 0.121262, -0.005274, 0.567662), i4[2])
                + dot(vec4(0.006045, -0.077373, -0.072533, -0.031636), i5[2])
                + dot(vec4(-0.147955, -0.338939, -0.097488, -0.004534), i6[2])
                + dot(vec4(0.040902, -0.545061, 0.029946, -0.206155), i7[2])
                + dot(vec4(-0.070122, -0.538919, -0.085261, 0.339183), i8[2]),
            dot(vec4(0.226298, 0.291284, -0.410465, -0.411055), i0[0])
                + dot(vec4(-0.050651, 0.158813, 0.095533, -0.055412), i1[0])
                + dot(vec4(0.341987, 0.133085, 0.308360, -0.137267), i2[0])
                + dot(vec4(0.285462, -0.119707, -0.339659, -0.278551), i3[0])
                + dot(vec4(-0.061771, 0.038256, -0.227301, 0.117410), i4[0])
                + dot(vec4(-0.031181, -0.209461, 0.453156, -0.118871), i5[0])
                + dot(vec4(0.372513, 0.165829, -0.065505, -0.313444), i6[0])
                + dot(vec4(0.128272, 0.180566, -0.431113, -0.160747), i7[0])
                + dot(vec4(0.337933, 0.394571, 0.503369, -0.400904), i8[0])
                + dot(vec4(-0.135610, -0.107199, -0.035469, 0.244765), i0[1])
                + dot(vec4(-0.066941, 0.024587, -0.037994, 0.232232), i1[1])
                + dot(vec4(-0.131191, -0.161863, 0.079994, 0.160731), i2[1])
                + dot(vec4(-0.068206, -0.059621, -0.174974, 0.125745), i3[1])
                + dot(vec4(0.099996, 0.118805, -0.013225, 0.016785), i4[1])
                + dot(vec4(0.141686, -0.213920, -0.148021, -0.188464), i5[1])
                + dot(vec4(-0.165375, -0.205597, -0.373981, -0.033179), i6[1])
                + dot(vec4(-0.066281, -0.149566, 0.457550, -0.186652), i7[1])
                + dot(vec4(-0.006624, -0.295070, -0.139486, -0.150032), i8[1])
                + dot(vec4(-0.153488, -0.354432, 0.146389, -0.285402), i0[2])
                + dot(vec4(-0.036005, 0.025143, -0.018368, 0.141622), i1[2])
                + dot(vec4(0.106602, 0.157298, 0.121103, 0.214986), i2[2])
                + dot(vec4(0.299121, -0.075988, 0.280246, 0.099058), i3[2])
                + dot(vec4(0.173196, 0.043055, -0.064607, 0.078216), i4[2])
                + dot(vec4(-0.035084, -0.020207, 0.222798, -0.027665), i5[2])
                + dot(vec4(-0.163343, 0.479559, 0.157049, 0.500926), i6[2])
                + dot(vec4(-0.354797, 0.116331, -0.058674, -0.083432), i7[2])
                + dot(vec4(-0.368567, -0.201440, 0.157691, -0.294890), i8[2]),
            dot(vec4(-0.210277, 0.149469, 0.293664, 0.274393), i0[0])
                + dot(vec4(-0.281596, -0.160622, -0.146859, 0.012657), i1[0])
                + dot(vec4(-0.236561, -0.152328, -0.241005, 0.315200), i2[0])
                + dot(vec4(-0.202762, 0.172945, 0.394560, 0.250077), i3[0])
                + dot(vec4(0.114750, 0.290268, 0.438959, 0.045436), i4[0])
                + dot(vec4(-0.171241, 0.026384, -0.680398, -0.073799), i5[0])
                + dot(vec4(-0.161741, -0.199887, 0.541109, 0.183855), i6[0])
                + dot(vec4(-0.122702, -0.257718, -0.101539, 0.199199), i7[0])
                + dot(vec4(-0.182477, -0.077578, -0.487019, 0.210446), i8[0])
                + dot(vec4(0.053148, 0.011766, 0.118121, -0.392413), i0[1])
                + dot(vec4(0.135435, -0.054100, -0.141193, -0.123249), i1[1])
                + dot(vec4(0.292036, 0.237785, 0.007183, -0.180597), i2[1])
                + dot(vec4(0.103621, 0.012685, 0.061198, 0.040195), i3[1])
                + dot(vec4(0.148349, -0.156638, -0.067149, 0.071653), i4[1])
                + dot(vec4(0.048221, 0.140697, 0.046974, 0.062430), i5[1])
                + dot(vec4(0.268621, -0.155871, 0.261375, 0.260464), i6[1])
                + dot(vec4(0.083819, 0.129891, -0.151589, 0.280080), i7[1])
                + dot(vec4(0.227486, 0.220246, -0.175186, 0.121744), i8[1])
                + dot(vec4(0.061408, 0.139497, -0.223819, 0.271592), i0[2])
                + dot(vec4(-0.170749, 0.121880, -0.140751, -0.099494), i1[2])
                + dot(vec4(-0.266058, -0.367293, -0.027975, -0.072705), i2[2])
                + dot(vec4(0.184151, -0.143076, -0.146141, -0.002425), i3[2])
                + dot(vec4(0.107264, -0.006166, -0.107936, 0.224398), i4[2])
                + dot(vec4(-0.066263, 0.079095, -0.055467, -0.120662), i5[2])
                + dot(vec4(-0.163975, -0.362929, -0.180186, -0.243675), i6[2])
                + dot(vec4(0.002487, 0.128667, -0.096664, 0.244343), i7[2])
                + dot(vec4(-0.141917, 0.241774, -0.142039, -0.081496), i8[2])
        ) + vec4(0.040212, -0.081362, 0.087175, 0.004508);
    
        }
    )
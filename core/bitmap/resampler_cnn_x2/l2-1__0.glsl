STRINGIFY(
        uniform sampler2D images[3];
        varying highp vec2 texCoord;
        uniform highp vec2 d1;

        lowp vec4 fetch(sampler2D image, highp float x, highp float y) {
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
    
        lowp vec4 i0[3], i1[3], i2[3], i3[3], i4[3], i5[3], i6[3], i7[3], i8[3];
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
            dot(vec4(-0.139366, 0.095074, -0.047440, 0.094433), i0[0])
                + dot(vec4(0.107059, 0.283902, 0.062197, -0.009525), i1[0])
                + dot(vec4(0.075559, -0.090224, 0.237829, -0.045756), i2[0])
                + dot(vec4(0.030084, 0.127627, -0.509676, -0.143006), i3[0])
                + dot(vec4(-0.278554, -0.243601, -0.087515, -0.032417), i4[0])
                + dot(vec4(0.087161, -0.033162, 0.029038, -0.027864), i5[0])
                + dot(vec4(-0.057916, -0.219001, 0.136260, -0.000539), i6[0])
                + dot(vec4(0.105140, 0.390563, -0.079258, 0.090428), i7[0])
                + dot(vec4(0.015395, -0.052464, 0.057905, 0.024839), i8[0])
                + dot(vec4(-0.147407, 0.032930, -0.106186, 0.196675), i0[1])
                + dot(vec4(-0.411581, -0.000643, 0.007347, 0.560744), i1[1])
                + dot(vec4(0.226118, 0.025375, 0.031405, 0.219875), i2[1])
                + dot(vec4(-0.141226, -0.036462, -0.149128, 0.264895), i3[1])
                + dot(vec4(0.328147, -0.072475, -0.036728, 0.337006), i4[1])
                + dot(vec4(0.103758, -0.053413, 0.044291, 0.267285), i5[1])
                + dot(vec4(0.021342, -0.001857, -0.038858, 0.148745), i6[1])
                + dot(vec4(-0.154372, 0.005711, -0.197489, 0.196448), i7[1])
                + dot(vec4(0.198836, 0.022764, 0.002339, 0.021552), i8[1])
                + dot(vec4(0.048735, -0.083311, -0.223379, 0.116152), i0[2])
                + dot(vec4(0.043038, 0.132049, 0.086586, -0.187474), i1[2])
                + dot(vec4(0.142232, -0.029546, -0.112239, -0.007172), i2[2])
                + dot(vec4(-0.063507, 0.053079, -0.158457, 0.368088), i3[2])
                + dot(vec4(0.516213, -0.068392, 0.202606, -0.297693), i4[2])
                + dot(vec4(-0.050940, 0.239374, -0.109645, 0.137138), i5[2])
                + dot(vec4(-0.625525, 0.067148, -0.015460, -0.123544), i6[2])
                + dot(vec4(0.164509, 0.058784, -0.241289, -0.099600), i7[2])
                + dot(vec4(0.134163, 0.056386, -0.043328, 0.092864), i8[2]),
            dot(vec4(0.205377, -0.026621, -0.199349, -0.139147), i0[0])
                + dot(vec4(-0.193443, -0.184482, -0.060981, 0.200251), i1[0])
                + dot(vec4(-0.189453, -0.002217, 0.259758, -0.042836), i2[0])
                + dot(vec4(-0.328283, 0.112332, -0.234526, -0.214939), i3[0])
                + dot(vec4(-1.956696, 0.118474, 0.093646, 0.175723), i4[0])
                + dot(vec4(-0.153881, 0.112467, 0.447089, 0.064368), i5[0])
                + dot(vec4(-0.097566, 0.032367, -0.219575, -0.022215), i6[0])
                + dot(vec4(-0.108436, -0.597779, 0.000967, -0.038579), i7[0])
                + dot(vec4(-0.330699, 0.174290, 0.043632, -0.111780), i8[0])
                + dot(vec4(0.062601, 0.053409, 0.079910, 0.232216), i0[1])
                + dot(vec4(-0.205972, -0.031332, -0.064909, 0.359655), i1[1])
                + dot(vec4(-0.077266, 0.098270, 0.110267, 0.251525), i2[1])
                + dot(vec4(0.419161, 0.089694, 0.147458, 0.156986), i3[1])
                + dot(vec4(-0.118279, 0.039828, -0.018893, 0.641857), i4[1])
                + dot(vec4(-0.057275, -0.028359, -0.009714, 0.135866), i5[1])
                + dot(vec4(0.155369, -0.135820, -0.211305, 0.008940), i6[1])
                + dot(vec4(-0.019128, -0.017586, -0.016964, 0.175448), i7[1])
                + dot(vec4(0.016412, 0.221958, -0.019876, 0.055255), i8[1])
                + dot(vec4(-0.000334, -0.221445, 0.115405, -0.159022), i0[2])
                + dot(vec4(-0.095107, -0.128515, -0.456772, 0.235191), i1[2])
                + dot(vec4(0.056748, -0.236945, -0.267035, 0.223143), i2[2])
                + dot(vec4(0.301757, -0.286937, 0.181874, -0.254108), i3[2])
                + dot(vec4(-0.402038, -0.157968, 0.005194, 0.118927), i4[2])
                + dot(vec4(0.165943, -0.203031, -0.230695, -0.370971), i5[2])
                + dot(vec4(0.641168, -0.224242, -0.355052, 0.251918), i6[2])
                + dot(vec4(-0.763640, -0.177698, -0.268718, -0.149479), i7[2])
                + dot(vec4(0.253968, -0.398267, -0.266211, 0.015481), i8[2]),
            dot(vec4(-0.106727, -0.138018, -0.013196, -0.101762), i0[0])
                + dot(vec4(-0.269780, -0.188446, 0.094326, 0.278444), i1[0])
                + dot(vec4(-0.308878, -0.010460, 0.047944, 0.256422), i2[0])
                + dot(vec4(-0.260004, 0.073176, -0.121750, 0.090378), i3[0])
                + dot(vec4(-0.113633, 0.056764, 0.069470, 0.100165), i4[0])
                + dot(vec4(-0.118557, 0.121270, 0.106562, 0.006617), i5[0])
                + dot(vec4(-0.035879, -0.132456, -0.023937, -0.123284), i6[0])
                + dot(vec4(-0.060349, -0.099564, -0.104273, -0.089804), i7[0])
                + dot(vec4(-0.052233, 0.257325, -0.019090, -0.137542), i8[0])
                + dot(vec4(-0.146104, -0.004702, -0.078880, 0.205102), i0[1])
                + dot(vec4(-0.279987, 0.015586, 0.009026, 0.543805), i1[1])
                + dot(vec4(-0.105048, -0.028962, 0.017541, -0.027737), i2[1])
                + dot(vec4(0.111727, -0.079392, -0.004665, 0.057095), i3[1])
                + dot(vec4(-0.012546, 0.065825, 0.024273, 0.410812), i4[1])
                + dot(vec4(0.012102, 0.002809, 0.048013, -0.007811), i5[1])
                + dot(vec4(0.141993, -0.290008, -0.017241, 0.119933), i6[1])
                + dot(vec4(0.077538, 0.222898, -0.034746, 0.208426), i7[1])
                + dot(vec4(0.176854, -0.058388, 0.172367, 0.066122), i8[1])
                + dot(vec4(0.108440, 0.040536, 0.049567, 0.112488), i0[2])
                + dot(vec4(0.060637, 0.005174, -0.114440, -0.083561), i1[2])
                + dot(vec4(0.138028, 0.087031, 0.114215, 0.183726), i2[2])
                + dot(vec4(0.018351, -0.009368, 0.048723, -0.127600), i3[2])
                + dot(vec4(-0.268315, 0.138965, 0.250905, -0.177615), i4[2])
                + dot(vec4(0.191954, 0.101766, -0.022638, -0.086184), i5[2])
                + dot(vec4(0.427813, 0.035336, 0.040682, 0.098558), i6[2])
                + dot(vec4(-0.165364, 0.271818, 0.047555, -0.088352), i7[2])
                + dot(vec4(0.033297, -0.032032, 0.071422, 0.138070), i8[2]),
            dot(vec4(0.006801, -0.109171, -0.029734, 0.139432), i0[0])
                + dot(vec4(0.172345, -0.173756, 0.139617, -0.067411), i1[0])
                + dot(vec4(0.213982, -0.021066, 0.051377, -0.041880), i2[0])
                + dot(vec4(-0.016036, 0.275519, -0.052273, 0.035884), i3[0])
                + dot(vec4(-0.065571, 0.108165, -0.133202, -0.008517), i4[0])
                + dot(vec4(0.176319, -0.247215, -0.247592, 0.021958), i5[0])
                + dot(vec4(-0.034044, 0.243095, -0.159949, 0.057975), i6[0])
                + dot(vec4(-0.062745, -0.077011, -0.086260, 0.035159), i7[0])
                + dot(vec4(0.115164, -0.092681, 0.136150, -0.034307), i8[0])
                + dot(vec4(0.104754, 0.074408, -0.023322, -0.055272), i0[1])
                + dot(vec4(0.184214, 0.016322, -0.054872, 0.447894), i1[1])
                + dot(vec4(0.065350, 0.104354, 0.200909, 0.288025), i2[1])
                + dot(vec4(0.017726, 0.148928, -0.127823, 0.156343), i3[1])
                + dot(vec4(-0.027821, -0.029951, 0.003788, 0.594375), i4[1])
                + dot(vec4(0.004465, -0.002628, 0.170599, 0.356663), i5[1])
                + dot(vec4(-0.027763, 0.125784, -0.039803, 0.024684), i6[1])
                + dot(vec4(-0.141296, -0.540421, -0.014537, 0.222227), i7[1])
                + dot(vec4(-0.018487, -0.012533, 0.330431, 0.219762), i8[1])
                + dot(vec4(-0.019706, -0.005986, 0.082176, -0.074846), i0[2])
                + dot(vec4(0.046088, 0.144517, -0.065482, 0.051895), i1[2])
                + dot(vec4(-0.171775, 0.082364, -0.084536, -0.067851), i2[2])
                + dot(vec4(0.701532, 0.015242, 0.062073, 0.023477), i3[2])
                + dot(vec4(-0.284542, 0.173720, 0.032267, 0.209214), i4[2])
                + dot(vec4(0.012005, 0.025400, 0.023714, -0.123014), i5[2])
                + dot(vec4(-0.370439, -0.055894, -0.175289, 0.059555), i6[2])
                + dot(vec4(0.431317, 0.055938, -0.176138, -0.183856), i7[2])
                + dot(vec4(-0.317103, 0.198809, 0.020929, 0.227845), i8[2])
        ) + vec4(0.168352, -0.005192, 0.081526, 0.081026);
    
        }
    )
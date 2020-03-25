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
            dot(vec4(0.258878, -0.191485, 0.123448, -0.049510), i0[0])
                + dot(vec4(0.285634, -0.461702, 0.003119, 0.077175), i1[0])
                + dot(vec4(0.056653, -0.149953, 0.181438, -0.209010), i2[0])
                + dot(vec4(0.028822, -0.179791, 0.065678, -0.013532), i3[0])
                + dot(vec4(0.012807, -0.295995, 0.088169, 0.183514), i4[0])
                + dot(vec4(0.062229, -0.287938, 0.049637, -0.247526), i5[0])
                + dot(vec4(0.136273, -0.039879, -0.046944, 0.088281), i6[0])
                + dot(vec4(0.143425, -0.199318, -0.000400, -0.384277), i7[0])
                + dot(vec4(0.124452, -0.264583, -0.004044, 0.095490), i8[0])
                + dot(vec4(-0.168400, -0.127001, 0.161860, -0.029502), i0[1])
                + dot(vec4(0.511038, -0.101436, 0.232584, -0.105752), i1[1])
                + dot(vec4(-0.015621, 0.146560, 0.150313, 0.086269), i2[1])
                + dot(vec4(-0.275589, 0.016478, 0.223614, 0.205757), i3[1])
                + dot(vec4(0.414670, -0.032077, 0.237322, 0.110814), i4[1])
                + dot(vec4(-0.207912, -0.148215, 0.095333, 0.053218), i5[1])
                + dot(vec4(-0.264736, 0.019326, 0.280645, -0.162991), i6[1])
                + dot(vec4(0.333355, 0.287242, 0.206266, -0.007881), i7[1])
                + dot(vec4(-0.287738, -0.003956, 0.059589, -0.222930), i8[1])
                + dot(vec4(0.044739, -0.213980, -0.243824, -0.131792), i0[2])
                + dot(vec4(0.033107, -0.131973, 0.104303, 0.029809), i1[2])
                + dot(vec4(0.045133, 0.069853, 0.013943, 0.037015), i2[2])
                + dot(vec4(0.039231, -0.159399, 0.356574, -0.233344), i3[2])
                + dot(vec4(0.060626, -0.133923, -0.086508, 0.036252), i4[2])
                + dot(vec4(-0.072942, 0.133680, -0.089813, 0.012494), i5[2])
                + dot(vec4(-0.076646, 0.012064, 0.173150, -0.068643), i6[2])
                + dot(vec4(0.055536, 0.117836, 0.015331, -0.042165), i7[2])
                + dot(vec4(0.036233, 0.046202, -0.041609, 0.027683), i8[2]),
            dot(vec4(-0.258742, -0.045983, -0.106040, 0.056518), i0[0])
                + dot(vec4(0.058047, 0.153221, 0.147511, 0.130407), i1[0])
                + dot(vec4(0.035282, 0.101810, 0.031748, 0.196076), i2[0])
                + dot(vec4(-0.072402, -0.149102, -0.000094, -0.156758), i3[0])
                + dot(vec4(-0.265878, 0.284224, -0.288687, 0.351987), i4[0])
                + dot(vec4(0.130384, -0.002700, -0.084881, 0.306093), i5[0])
                + dot(vec4(0.107717, -0.088555, 0.105898, 0.341864), i6[0])
                + dot(vec4(0.188339, 0.044532, -0.134515, -0.753295), i7[0])
                + dot(vec4(-0.013524, 0.115891, -0.031669, -0.843110), i8[0])
                + dot(vec4(0.075826, -0.019979, -0.356929, 0.236903), i0[1])
                + dot(vec4(0.312851, -0.083297, -0.069937, -0.133158), i1[1])
                + dot(vec4(-0.324620, -0.015736, -0.203471, 0.065532), i2[1])
                + dot(vec4(-0.047560, -0.152280, -0.072613, 0.141775), i3[1])
                + dot(vec4(-0.560181, 0.192197, -0.200503, 0.401922), i4[1])
                + dot(vec4(0.680521, 0.216213, 0.041507, 0.020613), i5[1])
                + dot(vec4(0.162124, -0.316589, -0.079565, 0.257844), i6[1])
                + dot(vec4(0.214943, 0.040070, -0.096572, -0.366693), i7[1])
                + dot(vec4(-0.439667, -0.204637, 0.051443, -0.213830), i8[1])
                + dot(vec4(0.074565, -0.548363, -0.043079, -0.263338), i0[2])
                + dot(vec4(0.011266, -0.583065, -0.033232, 0.024631), i1[2])
                + dot(vec4(-0.006881, -0.051759, 0.116625, 0.055916), i2[2])
                + dot(vec4(0.168303, -1.466407, 0.022971, 0.000966), i3[2])
                + dot(vec4(0.162354, 0.550686, 0.164763, 0.482658), i4[2])
                + dot(vec4(0.050886, 0.100232, 0.153321, -0.063598), i5[2])
                + dot(vec4(0.079260, -0.080804, -0.007368, -0.052289), i6[2])
                + dot(vec4(-0.147686, 0.435734, 0.192978, 0.064868), i7[2])
                + dot(vec4(0.023659, 0.197886, 0.068836, 0.039004), i8[2]),
            dot(vec4(-0.637212, 0.052470, 0.243407, -0.060866), i0[0])
                + dot(vec4(-0.979465, 0.562027, -0.455592, -0.386631), i1[0])
                + dot(vec4(-0.696676, -0.350957, 0.679416, -0.026377), i2[0])
                + dot(vec4(-0.398029, 0.105508, -0.578930, -0.146165), i3[0])
                + dot(vec4(-1.640596, 0.735789, 0.677146, 0.229160), i4[0])
                + dot(vec4(-0.429989, -0.328561, -0.673532, -1.598357), i5[0])
                + dot(vec4(-0.998386, 0.074754, 0.232423, -0.168628), i6[0])
                + dot(vec4(-0.311570, 0.119011, -0.390906, -1.252725), i7[0])
                + dot(vec4(0.100617, 1.070084, 0.304572, 0.137861), i8[0])
                + dot(vec4(-0.136878, 0.635264, 0.008792, 0.812657), i0[1])
                + dot(vec4(0.232687, 0.586494, 0.678788, 0.135129), i1[1])
                + dot(vec4(-0.164245, 0.495435, 0.191905, 0.662171), i2[1])
                + dot(vec4(-0.868414, 0.009221, -0.033226, -0.151486), i3[1])
                + dot(vec4(0.107992, -0.303624, -0.281808, -0.030484), i4[1])
                + dot(vec4(-0.098466, 0.327521, 0.392984, -0.242005), i5[1])
                + dot(vec4(-0.237480, 0.113879, -0.048792, 0.235264), i6[1])
                + dot(vec4(0.204154, -0.112139, -0.372840, 1.018482), i7[1])
                + dot(vec4(-1.167862, -0.525612, 0.195164, -1.048727), i8[1])
                + dot(vec4(-0.270707, 0.189225, -1.396015, 0.338373), i0[2])
                + dot(vec4(-1.127202, 0.081005, 0.518937, 0.529971), i1[2])
                + dot(vec4(-0.256319, 0.439977, 0.095227, -0.719201), i2[2])
                + dot(vec4(0.031179, 0.177873, -1.386549, 0.973133), i3[2])
                + dot(vec4(-1.519470, 0.443873, 0.140054, 0.152455), i4[2])
                + dot(vec4(0.150283, -0.336801, -0.011891, -0.020582), i5[2])
                + dot(vec4(0.163917, -0.101516, -0.028566, 0.185209), i6[2])
                + dot(vec4(-1.551007, -0.000109, -0.286797, -0.222124), i7[2])
                + dot(vec4(-0.282229, 0.557564, 0.879634, -0.103093), i8[2]),
            dot(vec4(0.022841, -0.029904, -0.367136, 0.012041), i0[0])
                + dot(vec4(0.132618, -0.042952, 0.076631, 0.116156), i1[0])
                + dot(vec4(0.193321, -0.063788, 0.011349, -0.073215), i2[0])
                + dot(vec4(0.037659, 0.081947, -0.092104, 0.164411), i3[0])
                + dot(vec4(0.002541, 0.094470, -0.037446, 0.007537), i4[0])
                + dot(vec4(0.134754, -0.003506, -0.014152, 0.156915), i5[0])
                + dot(vec4(0.045356, 0.071761, -0.056662, 0.488370), i6[0])
                + dot(vec4(-0.088022, 0.029534, 0.045133, -0.857912), i7[0])
                + dot(vec4(0.124860, -0.013803, 0.066709, 0.107496), i8[0])
                + dot(vec4(-0.022419, 0.049496, 0.083334, -0.038276), i0[1])
                + dot(vec4(-0.052379, 0.050934, 0.112508, -0.026467), i1[1])
                + dot(vec4(0.016761, 0.084571, 0.106696, 0.054333), i2[1])
                + dot(vec4(0.091844, -0.054932, 0.070709, -0.215906), i3[1])
                + dot(vec4(-0.235125, 0.151836, -0.009820, 0.154231), i4[1])
                + dot(vec4(-0.117058, -0.006126, 0.025769, 0.189326), i5[1])
                + dot(vec4(0.178564, 0.041785, 0.082939, -0.355069), i6[1])
                + dot(vec4(0.106536, -0.524193, 0.052922, -0.463588), i7[1])
                + dot(vec4(-0.119400, 0.104730, 0.035934, 0.120665), i8[1])
                + dot(vec4(-0.110783, 0.015626, 0.177479, 0.465105), i0[2])
                + dot(vec4(-0.044636, -0.037290, 0.250078, 0.027773), i1[2])
                + dot(vec4(-0.002733, -0.080522, 0.151036, -0.090701), i2[2])
                + dot(vec4(-0.193192, 0.862802, 0.013778, 0.939384), i3[2])
                + dot(vec4(-0.111866, 0.858970, 0.275277, -0.130435), i4[2])
                + dot(vec4(0.068698, -0.060341, 0.129692, -0.033464), i5[2])
                + dot(vec4(-0.246700, -0.109580, -0.028694, 0.090899), i6[2])
                + dot(vec4(-0.181421, -0.342304, 0.007183, -0.033257), i7[2])
                + dot(vec4(-0.015631, -0.077165, -0.045025, 0.043469), i8[2])
        ) + vec4(0.018807, -0.058005, -0.619714, 0.097628);
    
        }
    )
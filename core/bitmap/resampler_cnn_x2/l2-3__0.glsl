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
            dot(vec4(0.061022, -0.112579, 0.234571, 0.083538), i0[0])
                + dot(vec4(-0.216759, -0.110549, 0.088997, 0.158496), i1[0])
                + dot(vec4(0.170001, -0.066440, -0.124715, 0.138135), i2[0])
                + dot(vec4(-0.017849, -0.144019, -0.155231, 0.146942), i3[0])
                + dot(vec4(-0.011307, -0.029311, 0.195617, 0.064145), i4[0])
                + dot(vec4(-0.241227, 0.180036, 0.175660, 0.207599), i5[0])
                + dot(vec4(0.195758, -0.033642, -0.122296, 0.002066), i6[0])
                + dot(vec4(0.034107, 0.130460, -0.122828, -0.002396), i7[0])
                + dot(vec4(-0.060572, -0.123603, -0.096405, 0.160323), i8[0])
                + dot(vec4(0.065846, -0.210254, -0.022268, -0.159509), i0[1])
                + dot(vec4(0.645545, -0.123737, -0.070390, -0.256724), i1[1])
                + dot(vec4(-0.021263, -0.231158, 0.017264, 0.110020), i2[1])
                + dot(vec4(0.146399, 0.038956, 0.001493, 0.029224), i3[1])
                + dot(vec4(0.037616, -0.080047, 0.175998, 0.194392), i4[1])
                + dot(vec4(-0.301447, 0.043924, 0.116977, 0.123214), i5[1])
                + dot(vec4(0.054593, 0.052006, -0.036771, 0.014689), i6[1])
                + dot(vec4(0.038539, -0.025559, 0.001595, -0.140471), i7[1])
                + dot(vec4(-0.085683, -0.059090, 0.016282, -0.062151), i8[1])
                + dot(vec4(0.087088, 0.180586, -0.012779, 0.082953), i0[2])
                + dot(vec4(0.154549, 0.344966, -0.009193, 0.104478), i1[2])
                + dot(vec4(-0.037872, 0.285952, -0.087263, 0.074754), i2[2])
                + dot(vec4(-0.242191, 0.197000, -0.068302, 0.063344), i3[2])
                + dot(vec4(0.212766, 0.366917, -0.027619, 0.276595), i4[2])
                + dot(vec4(-0.000620, 0.382285, 0.023793, 0.057022), i5[2])
                + dot(vec4(-0.043772, 0.093859, 0.009990, 0.133323), i6[2])
                + dot(vec4(0.090909, -0.016020, 0.220969, 0.045483), i7[2])
                + dot(vec4(-0.070217, 0.295550, 0.182966, 0.193339), i8[2]),
            dot(vec4(0.002465, -0.037994, -0.071935, 0.083834), i0[0])
                + dot(vec4(-0.168857, -0.054826, 0.137616, 0.003017), i1[0])
                + dot(vec4(0.014849, 0.192801, -0.030280, -0.070591), i2[0])
                + dot(vec4(0.190954, 0.029335, 0.000961, 0.019362), i3[0])
                + dot(vec4(0.028940, -0.202509, 0.133466, 0.077390), i4[0])
                + dot(vec4(-0.151950, 0.189042, -0.180218, -0.035413), i5[0])
                + dot(vec4(-0.040344, 0.058657, -0.011332, -0.149526), i6[0])
                + dot(vec4(0.095019, -0.411972, 0.315839, 0.068250), i7[0])
                + dot(vec4(0.027859, 0.408385, -0.117997, 0.019861), i8[0])
                + dot(vec4(0.392914, 0.194487, 0.086397, 0.008831), i0[1])
                + dot(vec4(0.405429, -0.080960, -0.192272, -0.044202), i1[1])
                + dot(vec4(-0.416539, -0.133675, 0.001171, 0.056072), i2[1])
                + dot(vec4(0.476604, 0.065750, 0.375789, -0.089736), i3[1])
                + dot(vec4(0.408230, -0.201273, -0.598682, -0.325669), i4[1])
                + dot(vec4(-0.468895, -0.113200, 0.098549, 0.136472), i5[1])
                + dot(vec4(0.083383, 0.389195, 0.194330, -0.292300), i6[1])
                + dot(vec4(0.119164, -0.525337, -0.112720, 0.188870), i7[1])
                + dot(vec4(-0.142535, -0.089644, 0.126570, 0.066996), i8[1])
                + dot(vec4(0.016290, -0.047759, -0.055345, -0.111177), i0[2])
                + dot(vec4(-0.006337, -0.035167, 0.050148, -0.023921), i1[2])
                + dot(vec4(-0.052480, 0.164369, 0.121690, 0.065390), i2[2])
                + dot(vec4(-0.064835, -0.257300, 0.018036, 0.005286), i3[2])
                + dot(vec4(-0.289200, 0.304314, 0.010099, -0.180937), i4[2])
                + dot(vec4(0.251970, 0.033824, -0.171177, -0.054926), i5[2])
                + dot(vec4(0.282726, -0.054270, -0.088587, 0.306914), i6[2])
                + dot(vec4(-0.314789, 0.154118, -0.011437, -0.241965), i7[2])
                + dot(vec4(-0.134610, -0.047116, 0.061370, -0.087316), i8[2]),
            dot(vec4(0.246960, -0.076756, -0.253794, -0.018317), i0[0])
                + dot(vec4(-0.045505, 0.000777, 0.135785, -0.019679), i1[0])
                + dot(vec4(0.027063, 0.026927, -0.068585, -0.120957), i2[0])
                + dot(vec4(0.162523, 0.105289, 0.164390, -0.044306), i3[0])
                + dot(vec4(0.063132, 0.191763, 0.157455, -0.101888), i4[0])
                + dot(vec4(-0.033474, 0.153880, 0.008910, -0.147334), i5[0])
                + dot(vec4(0.065686, 0.015246, 0.046139, 0.183464), i6[0])
                + dot(vec4(-0.375906, -0.263757, -0.131137, -0.104752), i7[0])
                + dot(vec4(-0.046980, -0.137582, 0.045540, -0.047816), i8[0])
                + dot(vec4(0.122396, -0.009139, -0.005474, 0.013569), i0[1])
                + dot(vec4(-0.136200, -0.116114, 0.333303, 0.210991), i1[1])
                + dot(vec4(-0.564542, -0.082883, -0.063332, 0.134955), i2[1])
                + dot(vec4(0.013758, 0.299744, 0.211927, -0.049451), i3[1])
                + dot(vec4(0.824502, 0.207669, -0.268282, -0.212802), i4[1])
                + dot(vec4(0.963723, -0.081587, -0.166514, 0.187276), i5[1])
                + dot(vec4(-0.022733, -0.120152, -0.018588, -0.028684), i6[1])
                + dot(vec4(0.194960, -0.191264, 0.193453, 0.110612), i7[1])
                + dot(vec4(0.008374, -0.109044, 0.120347, 0.097010), i8[1])
                + dot(vec4(-0.057686, 0.013561, -0.036398, -0.033816), i0[2])
                + dot(vec4(-0.065978, -0.140704, 0.029746, -0.048512), i1[2])
                + dot(vec4(0.024498, -0.194230, 0.164072, 0.059432), i2[2])
                + dot(vec4(0.094959, -0.013926, 0.274865, 0.150799), i3[2])
                + dot(vec4(0.034894, 0.245342, 0.167389, -0.227809), i4[2])
                + dot(vec4(-0.133504, -0.121647, 0.008261, -0.266767), i5[2])
                + dot(vec4(0.161659, -0.033631, -0.076227, 0.140822), i6[2])
                + dot(vec4(-0.050229, 0.172787, 0.011805, 0.182679), i7[2])
                + dot(vec4(-0.009292, 0.345722, -0.325215, -0.015340), i8[2]),
            dot(vec4(-0.143902, -0.081262, 0.169933, -0.068023), i0[0])
                + dot(vec4(0.324893, -0.089692, 0.067148, 0.044682), i1[0])
                + dot(vec4(0.000690, -0.157624, 0.066159, -0.088131), i2[0])
                + dot(vec4(-0.388450, -0.047368, -0.044566, -0.108659), i3[0])
                + dot(vec4(0.216736, 0.097025, -0.111987, 0.015271), i4[0])
                + dot(vec4(0.301578, 0.334091, 0.051211, -0.105104), i5[0])
                + dot(vec4(-0.265575, 0.049478, -0.220710, 0.018354), i6[0])
                + dot(vec4(0.253567, -0.122135, -0.086896, -0.075737), i7[0])
                + dot(vec4(0.120437, -0.073257, 0.148696, -0.025804), i8[0])
                + dot(vec4(0.264046, -0.032811, 0.160229, -0.083396), i0[1])
                + dot(vec4(0.230880, -0.129405, 0.154411, 0.051861), i1[1])
                + dot(vec4(-0.512728, -0.254709, 0.081231, 0.275789), i2[1])
                + dot(vec4(0.185553, 0.130834, -0.040426, -0.108583), i3[1])
                + dot(vec4(0.590376, 0.138049, 0.173746, 0.112214), i4[1])
                + dot(vec4(0.379448, 0.030547, -0.116500, 0.006636), i5[1])
                + dot(vec4(0.040349, -0.015959, 0.034692, 0.014067), i6[1])
                + dot(vec4(0.157977, 0.063874, -0.123268, 0.074116), i7[1])
                + dot(vec4(0.014924, -0.045730, -0.107385, 0.132341), i8[1])
                + dot(vec4(-0.036488, 0.107410, -0.027884, 0.144080), i0[2])
                + dot(vec4(-0.051418, 0.005640, -0.002625, 0.166490), i1[2])
                + dot(vec4(0.139515, 0.185198, 0.081590, 0.186073), i2[2])
                + dot(vec4(-0.142437, 0.030773, -0.115470, 0.246921), i3[2])
                + dot(vec4(0.075239, 0.188543, 0.062100, -0.075566), i4[2])
                + dot(vec4(-0.067107, -0.119428, -0.094505, -0.153574), i5[2])
                + dot(vec4(0.132362, 0.010130, -0.284097, 0.192321), i6[2])
                + dot(vec4(0.170212, 0.144092, 0.124752, 0.200058), i7[2])
                + dot(vec4(-0.119502, 0.273207, -0.091031, -0.013877), i8[2])
        ) + vec4(0.028941, 0.028980, 0.176369, 0.089975);
    
        }
    )
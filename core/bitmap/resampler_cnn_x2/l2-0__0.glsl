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
            dot(vec4(-0.037510, -0.129684, -0.136833, 0.254396), i0[0])
                + dot(vec4(-0.008796, -0.060837, 0.084187, 0.110192), i1[0])
                + dot(vec4(-0.072814, 0.016363, 0.043563, 0.111566), i2[0])
                + dot(vec4(0.238702, 0.080421, 0.105921, 0.079749), i3[0])
                + dot(vec4(0.173742, -0.264879, -0.104369, -0.184818), i4[0])
                + dot(vec4(0.220172, -0.055783, 0.038904, 0.002133), i5[0])
                + dot(vec4(-0.068224, 0.273287, 0.055438, 0.374564), i6[0])
                + dot(vec4(0.053998, -0.050122, 0.004545, -0.261035), i7[0])
                + dot(vec4(-0.085494, 0.165153, -0.048676, -0.346616), i8[0])
                + dot(vec4(0.229055, -0.134501, 0.030910, -0.141624), i0[1])
                + dot(vec4(0.320070, 0.157117, 0.229389, -0.132595), i1[1])
                + dot(vec4(-0.028301, -0.008827, 0.104382, 0.002430), i2[1])
                + dot(vec4(0.065508, -0.040204, 0.108087, -0.037569), i3[1])
                + dot(vec4(-0.118884, 0.085052, 0.350571, 0.021512), i4[1])
                + dot(vec4(0.031292, -0.135700, -0.198697, -0.118613), i5[1])
                + dot(vec4(-0.108994, -0.025949, -0.261586, 0.044120), i6[1])
                + dot(vec4(-0.043143, -0.123376, 0.171139, 0.039696), i7[1])
                + dot(vec4(0.012461, 0.004007, -0.130038, -0.140898), i8[1])
                + dot(vec4(0.465929, 0.061048, -0.066712, -0.067891), i0[2])
                + dot(vec4(0.510155, 0.032544, -0.121327, 0.004668), i1[2])
                + dot(vec4(-0.164501, -0.068430, -0.031017, 0.073807), i2[2])
                + dot(vec4(0.865590, -0.152668, -0.048340, -0.044223), i3[2])
                + dot(vec4(0.138144, -0.177148, -0.003326, -0.066594), i4[2])
                + dot(vec4(-0.084586, -0.157279, 0.106344, 0.057257), i5[2])
                + dot(vec4(-0.075883, 0.067061, -0.079530, -0.050723), i6[2])
                + dot(vec4(-0.158658, 0.204441, -0.025147, -0.054507), i7[2])
                + dot(vec4(-0.147501, 0.294515, -0.003420, -0.005604), i8[2]),
            dot(vec4(-0.098844, -0.187088, -0.236229, 0.066200), i0[0])
                + dot(vec4(0.311144, 0.073554, -0.085652, -0.187275), i1[0])
                + dot(vec4(0.003674, 0.026767, 0.158680, -0.038801), i2[0])
                + dot(vec4(0.147966, -0.151741, 0.273192, -0.075110), i3[0])
                + dot(vec4(-0.049767, -0.364310, -0.336533, 0.143113), i4[0])
                + dot(vec4(0.067283, 0.397637, 0.100228, -0.107018), i5[0])
                + dot(vec4(-0.037289, -0.061560, 0.281434, 0.139902), i6[0])
                + dot(vec4(-0.136969, -0.073936, 0.088914, -0.082013), i7[0])
                + dot(vec4(-0.089494, 0.183509, -0.011179, 0.085168), i8[0])
                + dot(vec4(0.164921, 0.613709, -0.308623, -0.132187), i0[1])
                + dot(vec4(-0.094528, 0.284846, 0.253933, -0.075122), i1[1])
                + dot(vec4(0.007782, 0.780351, 0.241823, 0.028607), i2[1])
                + dot(vec4(0.003746, 0.380219, -0.258334, 0.125419), i3[1])
                + dot(vec4(0.290721, -0.204407, -0.091711, 0.036934), i4[1])
                + dot(vec4(0.194157, 0.464840, -0.181257, 0.030840), i5[1])
                + dot(vec4(-0.169939, 0.522905, -0.083526, 0.192422), i6[1])
                + dot(vec4(-0.004639, 0.164891, 0.226630, -0.131024), i7[1])
                + dot(vec4(0.164996, 0.164086, 0.313183, -0.143391), i8[1])
                + dot(vec4(0.058394, -0.121215, -0.345109, 0.169876), i0[2])
                + dot(vec4(0.025650, -0.207130, -0.145906, 0.161291), i1[2])
                + dot(vec4(0.058534, -0.153512, -0.496803, 0.260955), i2[2])
                + dot(vec4(-0.181390, 0.021430, -0.262604, 0.275204), i3[2])
                + dot(vec4(-0.160830, -0.043585, -0.187623, 0.046643), i4[2])
                + dot(vec4(-0.080764, -0.244961, -0.277909, 0.235427), i5[2])
                + dot(vec4(-0.005267, -0.151438, -0.371668, 0.276885), i6[2])
                + dot(vec4(0.132705, -0.312502, -0.394445, 0.298365), i7[2])
                + dot(vec4(0.092362, 0.154921, -0.373554, 0.438627), i8[2]),
            dot(vec4(0.095477, 0.074651, -0.015112, -0.063899), i0[0])
                + dot(vec4(0.185376, 0.101665, -0.110626, -0.051948), i1[0])
                + dot(vec4(0.076510, -0.083990, 0.141599, -0.183107), i2[0])
                + dot(vec4(-0.147860, -0.048411, -0.108128, 0.092231), i3[0])
                + dot(vec4(0.166765, 0.078519, -0.107085, 0.016794), i4[0])
                + dot(vec4(0.125909, 0.033742, 0.240422, 0.034161), i5[0])
                + dot(vec4(0.110965, -0.124544, 0.052147, -0.070779), i6[0])
                + dot(vec4(-0.168155, -0.024368, 0.101396, 0.025557), i7[0])
                + dot(vec4(0.087761, 0.012037, -0.035791, 0.043876), i8[0])
                + dot(vec4(0.109600, -0.136709, -0.001071, 0.082149), i0[1])
                + dot(vec4(-0.202961, -0.198965, -0.029608, 0.156574), i1[1])
                + dot(vec4(0.069599, -0.124536, -0.049345, 0.054377), i2[1])
                + dot(vec4(0.086085, -0.213408, -0.151369, 0.012989), i3[1])
                + dot(vec4(-0.049222, -0.155585, 0.114811, -0.045327), i4[1])
                + dot(vec4(0.132211, -0.288390, -0.050767, 0.124802), i5[1])
                + dot(vec4(0.193433, -0.324940, 0.152419, 0.103276), i6[1])
                + dot(vec4(-0.019536, -0.058695, 0.076756, -0.067464), i7[1])
                + dot(vec4(0.023708, -0.000150, 0.039878, 0.187058), i8[1])
                + dot(vec4(-0.139743, 0.012125, 0.043209, -0.054283), i0[2])
                + dot(vec4(-0.229644, 0.039620, -0.049492, 0.004328), i1[2])
                + dot(vec4(0.031909, -0.016004, 0.012483, -0.171327), i2[2])
                + dot(vec4(0.290275, -0.009192, 0.059312, -0.159247), i3[2])
                + dot(vec4(-0.101992, 0.053793, -0.019419, -0.033265), i4[2])
                + dot(vec4(-0.141471, 0.156308, -0.003481, -0.113220), i5[2])
                + dot(vec4(-0.010252, -0.007096, -0.002108, -0.174068), i6[2])
                + dot(vec4(0.086073, -0.071327, -0.068396, -0.136765), i7[2])
                + dot(vec4(0.007024, 0.044491, 0.060382, -0.026297), i8[2]),
            dot(vec4(0.005932, -0.157134, 0.053059, 0.051308), i0[0])
                + dot(vec4(-0.040872, 0.044883, -0.035153, 0.315256), i1[0])
                + dot(vec4(-0.036079, 0.013480, 0.158000, 0.075761), i2[0])
                + dot(vec4(0.004554, -0.095189, 0.001522, 0.139218), i3[0])
                + dot(vec4(0.043177, -0.077613, -0.275240, -0.580156), i4[0])
                + dot(vec4(-0.033782, 0.053280, -0.069349, 0.153089), i5[0])
                + dot(vec4(-0.013110, 0.081194, 0.254952, -0.000917), i6[0])
                + dot(vec4(0.180257, -0.006513, 0.073709, 0.256078), i7[0])
                + dot(vec4(0.202489, -0.014160, -0.139583, -0.146359), i8[0])
                + dot(vec4(0.097955, -0.085035, 0.126738, -0.001824), i0[1])
                + dot(vec4(0.296631, -0.088274, 0.240155, -0.017931), i1[1])
                + dot(vec4(0.082097, 0.000070, 0.037072, -0.123616), i2[1])
                + dot(vec4(0.115331, -0.114688, -0.163056, 0.123175), i3[1])
                + dot(vec4(0.103863, -0.018660, -0.131158, 0.128337), i4[1])
                + dot(vec4(-0.071858, -0.020609, 0.279934, 0.083045), i5[1])
                + dot(vec4(-0.061367, 0.057775, -0.212901, 0.075675), i6[1])
                + dot(vec4(-0.039776, 0.064130, 0.214906, -0.113827), i7[1])
                + dot(vec4(0.059644, 0.171742, 0.176518, -0.063685), i8[1])
                + dot(vec4(0.197989, 0.006903, 0.005318, 0.006472), i0[2])
                + dot(vec4(0.075818, -0.033136, 0.001088, -0.067202), i1[2])
                + dot(vec4(0.054862, -0.031885, -0.045144, -0.000765), i2[2])
                + dot(vec4(0.047151, 0.054592, -0.059509, -0.061328), i3[2])
                + dot(vec4(-0.258353, 0.049115, -0.024350, -0.072878), i4[2])
                + dot(vec4(0.038715, -0.108408, 0.004470, -0.005806), i5[2])
                + dot(vec4(0.008256, -0.011783, -0.040959, -0.002893), i6[2])
                + dot(vec4(-0.109630, -0.088728, -0.041724, 0.069882), i7[2])
                + dot(vec4(-0.068028, -0.018459, -0.035661, 0.074677), i8[2])
        ) + vec4(0.107129, -0.052006, 0.027809, 0.089734);
    
        }
    )
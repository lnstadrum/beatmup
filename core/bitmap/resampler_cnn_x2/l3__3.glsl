STRINGIFY(
        uniform sampler2D images[8];
        varying highp vec2 texCoord;

        void main() {
            
        vec4 f[8];
        for (int i = 0; i < 8; ++i)
            f[i] = texture2D(images[i], texCoord);
    
        gl_FragColor = vec4(
            dot(vec4(0.488405, -0.030596, 0.078126, -0.220890), f[0])
                + dot(vec4(0.034381, 0.077394, -0.199065, 0.049351), f[1])
                + dot(vec4(0.060892, 0.260047, 0.123098, 0.327346), f[2])
                + dot(vec4(-0.382613, 0.022143, -0.299409, -0.048842), f[3])
                + dot(vec4(0.291672, 0.005893, -0.177130, -0.503067), f[4])
                + dot(vec4(-0.037697, 0.290785, 0.030809, 0.045272), f[5])
                + dot(vec4(-0.273878, -0.053387, -0.030862, 0.234354), f[6])
                + dot(vec4(0.197421, -0.021501, 0.037825, -1.035957), f[7]),
            dot(vec4(0.208310, -0.237740, -0.088678, -0.037973), f[0])
                + dot(vec4(0.290602, -0.251021, 0.049435, 0.052688), f[1])
                + dot(vec4(-0.258410, 0.102378, -0.207216, 0.092501), f[2])
                + dot(vec4(0.106870, -0.006727, 0.366130, 0.174045), f[3])
                + dot(vec4(0.041330, 0.126599, 0.031011, -0.093349), f[4])
                + dot(vec4(0.034482, -0.165210, 0.232293, -0.038340), f[5])
                + dot(vec4(0.178699, 0.034688, 0.070471, -0.444036), f[6])
                + dot(vec4(-0.081505, 0.010892, 0.112950, -0.436831), f[7]),
            dot(vec4(0.050243, -0.326979, -0.179492, -0.006019), f[0])
                + dot(vec4(-0.293694, -0.125197, 0.281420, 0.006070), f[1])
                + dot(vec4(-0.423656, -0.018974, -0.073594, 0.012413), f[2])
                + dot(vec4(0.349396, 0.517597, 0.139643, -0.264685), f[3])
                + dot(vec4(-0.313837, 0.140718, -0.296520, 0.095859), f[4])
                + dot(vec4(0.178001, 0.131078, 0.323397, -0.119026), f[5])
                + dot(vec4(0.228530, 0.217858, -0.040678, -0.055044), f[6])
                + dot(vec4(-0.300228, -0.125421, -0.066217, 0.439071), f[7]),
            dot(vec4(0.008597, -0.026439, 0.051715, 0.029736), f[0])
                + dot(vec4(0.066198, -0.234252, 0.194152, 0.074010), f[1])
                + dot(vec4(-0.364183, -0.088425, -0.157672, 0.040583), f[2])
                + dot(vec4(-0.249383, 0.202579, 0.297038, 0.002650), f[3])
                + dot(vec4(-0.162636, 0.047628, 0.106088, -0.269329), f[4])
                + dot(vec4(0.035198, 0.047413, 0.124260, -0.078930), f[5])
                + dot(vec4(0.201650, 0.224893, -0.016564, 0.121154), f[6])
                + dot(vec4(-0.120937, -0.039982, -0.086034, 0.343146), f[7])
        ) + vec4(-0.000154, 0.035379, 0.033254, 0.083366);
    
        }
    )
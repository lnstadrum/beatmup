STRINGIFY(
        uniform sampler2D images[4];
        varying highp vec2 texCoord;

        void main() {
            
        lowp vec4 f[4];
        for (int i = 0; i < 4; ++i)
            f[i] = texture2D(images[i], texCoord);
    
        gl_FragColor = vec4(
            dot(vec4(0.096580, 0.395025, -0.133056, -0.114765), f[0])
                + dot(vec4(0.171405, 0.131712, -0.070977, 0.813896), f[1])
                + dot(vec4(-0.080033, 0.117212, -0.143186, 0.141234), f[2])
                + dot(vec4(0.160355, 0.132391, -0.105776, -0.159369), f[3]),
            dot(vec4(-0.108157, 0.359289, 0.410946, -0.120859), f[0])
                + dot(vec4(0.183476, 0.109455, 0.092533, 0.493050), f[1])
                + dot(vec4(-0.099365, -0.116901, -0.137044, -0.277363), f[2])
                + dot(vec4(-0.160785, 0.106970, 0.128669, 0.137402), f[3]),
            dot(vec4(0.082581, 0.784930, 0.053892, 0.100896), f[0])
                + dot(vec4(-0.174872, -0.087598, -0.093985, 0.351007), f[1])
                + dot(vec4(0.090993, 0.078319, 0.150334, -0.107407), f[2])
                + dot(vec4(0.165042, -0.211887, -0.125771, -0.137847), f[3]),
            dot(vec4(-0.090314, 0.109329, 0.247851, 0.118906), f[0])
                + dot(vec4(-0.167901, -0.123229, 0.080445, 0.796893), f[1])
                + dot(vec4(0.083466, -0.100638, 0.127455, 0.239081), f[2])
                + dot(vec4(-0.163464, -0.030672, 0.088903, 0.150617), f[3])
        ) + vec4(-0.185612, -0.005043, 0.031067, -0.132663);
    
        }
    )
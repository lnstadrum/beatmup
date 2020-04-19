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
            
        lowp vec4 i0, i1, i2, i3, i4, i5, i6, i7, i8;
        mediump vec4 sum;
    
            i0 = fetch(images[0], x0, y0);
            i1 = fetch(images[0], x1, y0);
            i2 = fetch(images[0], x2, y0);
            i3 = fetch(images[0], x0, y1);
            i4 = fetch(images[0], x1, y1);
            i5 = fetch(images[0], x2, y1);
            i6 = fetch(images[0], x0, y2);
            i7 = fetch(images[0], x1, y2);
            i8 = fetch(images[0], x2, y2);
        
            sum = vec4(
                dot(vec4(0.037721, -0.189928, 0.080699, -0.135937), i0)
                + dot(vec4(-0.111442, -0.139167, -0.234687, 0.128234), i1)
                + dot(vec4(0.105587, -0.024194, -0.015468, -0.194529), i2)
                + dot(vec4(-0.131963, 0.051468, 0.104293, 0.072030), i3)
                + dot(vec4(-0.186151, 0.066437, -0.229832, -0.008626), i4)
                + dot(vec4(0.014370, -0.100692, -0.008843, -0.101912), i5)
                + dot(vec4(0.101255, -0.159723, -0.023162, 0.160206), i6)
                + dot(vec4(-0.054776, 0.060149, -0.006977, -0.039779), i7)
                + dot(vec4(-0.056363, -0.138087, 0.080826, 0.024266), i8),
                dot(vec4(-0.089317, -0.032517, 0.171229, -0.000612), i0)
                + dot(vec4(0.150555, -0.144625, -0.193187, 0.023849), i1)
                + dot(vec4(-0.143499, -0.097843, -0.047557, -0.143758), i2)
                + dot(vec4(-0.065988, 0.258819, 0.241395, 0.028672), i3)
                + dot(vec4(0.039558, -0.079518, -0.014998, -0.084659), i4)
                + dot(vec4(0.046230, -0.324474, 0.067140, -0.117688), i5)
                + dot(vec4(0.010424, 0.123058, 0.170420, 0.108490), i6)
                + dot(vec4(0.045655, 0.240505, 0.105707, 0.254706), i7)
                + dot(vec4(-0.029028, -0.048838, 0.082971, 0.037361), i8),
                dot(vec4(0.087576, -0.087991, -0.108246, 0.072007), i0)
                + dot(vec4(-0.043141, -0.092915, 0.485033, 0.086147), i1)
                + dot(vec4(0.073664, -0.138165, -0.146024, 0.035274), i2)
                + dot(vec4(0.123976, 0.116004, 0.075237, -0.012385), i3)
                + dot(vec4(-0.109544, 0.066629, -0.109237, -0.388335), i4)
                + dot(vec4(0.154001, 0.004183, -0.136092, 0.161572), i5)
                + dot(vec4(0.140874, 0.019176, -0.318311, 0.013866), i6)
                + dot(vec4(-0.394678, 0.056996, 0.459538, -0.143752), i7)
                + dot(vec4(0.108003, 0.144678, -0.077040, -0.007270), i8),
                dot(vec4(-0.077591, -0.004808, 0.213622, -0.088459), i0)
                + dot(vec4(0.087278, -0.008029, 0.131686, -0.084979), i1)
                + dot(vec4(-0.114346, 0.103266, 0.209561, -0.054848), i2)
                + dot(vec4(0.009350, -0.015647, 0.079195, 0.009781), i3)
                + dot(vec4(-0.090647, -0.176967, -0.161113, 0.452945), i4)
                + dot(vec4(0.112676, -0.233017, -0.188665, -0.517553), i5)
                + dot(vec4(-0.141285, 0.138638, 0.106766, 0.013164), i6)
                + dot(vec4(-0.103488, 0.050510, 0.157605, -0.171940), i7)
                + dot(vec4(0.217823, -0.297556, 0.139365, -0.120002), i8)
            );
        
            i0 = fetch(images[1], x0, y0);
            i1 = fetch(images[1], x1, y0);
            i2 = fetch(images[1], x2, y0);
            i3 = fetch(images[1], x0, y1);
            i4 = fetch(images[1], x1, y1);
            i5 = fetch(images[1], x2, y1);
            i6 = fetch(images[1], x0, y2);
            i7 = fetch(images[1], x1, y2);
            i8 = fetch(images[1], x2, y2);
        
            sum += vec4(
                dot(vec4(-0.048250, -0.107577, -0.060258, 0.103599), i0)
                + dot(vec4(0.029475, 0.026406, -0.190433, -0.178843), i1)
                + dot(vec4(0.083033, -0.045322, -0.232626, -0.121490), i2)
                + dot(vec4(0.167546, 0.136926, -0.123596, -0.149378), i3)
                + dot(vec4(-0.053634, 0.084631, 0.057026, -0.159986), i4)
                + dot(vec4(-0.039079, -0.163727, -0.208957, 0.094772), i5)
                + dot(vec4(0.001591, 0.138518, 0.064232, 0.067497), i6)
                + dot(vec4(-0.077507, -0.199004, -0.084665, 0.126785), i7)
                + dot(vec4(0.164892, 0.037179, -0.224468, 0.136489), i8),
                dot(vec4(0.061959, -0.212713, -0.281365, -0.008849), i0)
                + dot(vec4(0.010928, -0.068013, -0.084326, -0.087622), i1)
                + dot(vec4(0.021541, 0.022897, 0.013478, -0.067902), i2)
                + dot(vec4(0.145408, 0.239292, -0.203649, -0.093272), i3)
                + dot(vec4(0.235815, 0.009035, 0.116572, 0.191067), i4)
                + dot(vec4(0.019651, 0.034926, 0.009645, 0.082763), i5)
                + dot(vec4(0.082138, 0.132445, -0.020112, 0.020709), i6)
                + dot(vec4(0.087599, 0.001303, -0.099929, -0.116464), i7)
                + dot(vec4(0.068885, -0.012313, -0.045582, 0.100735), i8),
                dot(vec4(0.217855, -0.249556, 0.160873, 0.024152), i0)
                + dot(vec4(0.107459, -0.116962, 0.107549, 0.053535), i1)
                + dot(vec4(0.321198, -0.051199, -0.009760, -0.022125), i2)
                + dot(vec4(0.237238, 0.350263, -0.152327, 0.003316), i3)
                + dot(vec4(0.175379, 0.185310, 0.101405, -0.202762), i4)
                + dot(vec4(0.429149, 0.056582, -0.022705, -0.066525), i5)
                + dot(vec4(0.107142, -0.079751, 0.050131, 0.003474), i6)
                + dot(vec4(-0.024369, -0.000678, 0.007582, 0.139616), i7)
                + dot(vec4(0.168046, -0.052555, 0.023054, 0.106531), i8),
                dot(vec4(-0.003107, 0.474437, 0.834059, -0.108702), i0)
                + dot(vec4(-0.052455, 0.348514, 0.820517, 0.106149), i1)
                + dot(vec4(0.033442, 0.153114, 0.100587, 0.011341), i2)
                + dot(vec4(0.076723, 0.208101, 0.370908, 0.096712), i3)
                + dot(vec4(0.113347, -0.325573, 0.227922, -0.037044), i4)
                + dot(vec4(0.272053, -0.139193, 0.071074, 0.017861), i5)
                + dot(vec4(0.168558, -0.040913, 0.009341, 0.078849), i6)
                + dot(vec4(0.076310, 0.020667, 0.033729, -0.145311), i7)
                + dot(vec4(0.447834, -0.019966, 0.002707, 0.069171), i8)
            );
        
            i0 = fetch(images[2], x0, y0);
            i1 = fetch(images[2], x1, y0);
            i2 = fetch(images[2], x2, y0);
            i3 = fetch(images[2], x0, y1);
            i4 = fetch(images[2], x1, y1);
            i5 = fetch(images[2], x2, y1);
            i6 = fetch(images[2], x0, y2);
            i7 = fetch(images[2], x1, y2);
            i8 = fetch(images[2], x2, y2);
        
            sum += vec4(
                dot(vec4(-0.172802, 0.063090, -0.134139, -0.096242), i0)
                + dot(vec4(-0.185444, 0.027037, 0.038703, -0.057025), i1)
                + dot(vec4(-0.055654, 0.082064, 0.037425, 0.108187), i2)
                + dot(vec4(-0.104739, 0.014284, -0.070818, 0.007574), i3)
                + dot(vec4(-0.207032, -0.054126, -0.103277, 0.030230), i4)
                + dot(vec4(-0.013096, -0.127813, -0.030693, -0.189734), i5)
                + dot(vec4(-0.123618, -0.167771, -0.034045, -0.053667), i6)
                + dot(vec4(0.053320, -0.061704, -0.177032, -0.015498), i7)
                + dot(vec4(-0.197056, 0.220435, -0.010431, -0.181018), i8),
                dot(vec4(-0.093405, 0.028352, -0.011906, 0.090493), i0)
                + dot(vec4(-0.066696, 0.119336, -0.026139, -0.126566), i1)
                + dot(vec4(0.033886, 0.105098, -0.078220, 0.061228), i2)
                + dot(vec4(-0.224673, 0.021724, 0.125189, 0.263647), i3)
                + dot(vec4(-0.073839, -0.032305, -0.134092, 0.294642), i4)
                + dot(vec4(0.128242, -0.099416, 0.124815, 0.074102), i5)
                + dot(vec4(-0.036154, 0.010297, 0.114304, 0.137622), i6)
                + dot(vec4(0.184306, 0.177455, 0.129644, 0.176956), i7)
                + dot(vec4(-0.013711, 0.242404, 0.058345, 0.179742), i8),
                dot(vec4(-0.169314, -0.307693, -0.044737, -0.049431), i0)
                + dot(vec4(-0.152202, -0.178283, -0.104237, 0.026748), i1)
                + dot(vec4(-0.152230, -0.395206, 0.117819, 0.000865), i2)
                + dot(vec4(-0.374780, -0.394215, 0.446384, 0.110987), i3)
                + dot(vec4(-0.216538, -0.060265, 0.265299, 0.082536), i4)
                + dot(vec4(-0.158905, -0.370711, -0.104042, 0.187532), i5)
                + dot(vec4(0.012912, -0.395766, -0.264643, -0.035281), i6)
                + dot(vec4(0.154311, -0.159507, 0.171938, -0.000998), i7)
                + dot(vec4(0.036960, -0.337452, 0.053336, 0.056660), i8),
                dot(vec4(-0.060379, -0.208174, 0.137319, 0.121572), i0)
                + dot(vec4(0.011939, -0.040502, 0.232505, 0.028663), i1)
                + dot(vec4(0.025171, -0.096955, -0.556900, 0.201668), i2)
                + dot(vec4(0.125137, 0.117909, -0.317945, 0.153719), i3)
                + dot(vec4(0.184033, 0.339607, -0.122245, -0.127714), i4)
                + dot(vec4(-0.046983, 0.111641, -0.150648, -0.128187), i5)
                + dot(vec4(0.103816, -0.012758, 0.243073, 0.126332), i6)
                + dot(vec4(-0.081578, -0.306807, 0.061931, 0.010277), i7)
                + dot(vec4(-0.002159, 0.003038, 0.056581, -0.052704), i8)
            );
        
        gl_FragColor = sum + vec4(-0.022083, 0.130310, -0.114636, 0.030590);
    
        }
    )

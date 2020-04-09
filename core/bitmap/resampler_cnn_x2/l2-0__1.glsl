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
        highp vec4 sum;
    
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
                dot(vec4(0.009998, 0.017718, -0.050800, 0.120710), i0)
                + dot(vec4(0.120583, 0.084804, -0.109013, -0.112553), i1)
                + dot(vec4(0.117347, 0.049979, -0.026797, -0.052286), i2)
                + dot(vec4(0.107119, -0.126086, 0.129972, -0.154834), i3)
                + dot(vec4(0.048685, 0.072695, -0.248054, 0.421286), i4)
                + dot(vec4(0.130719, 0.079082, -0.024476, 0.004837), i5)
                + dot(vec4(0.128345, -0.283345, 0.036229, 0.199026), i6)
                + dot(vec4(0.094880, -0.044782, -0.005818, 0.152129), i7)
                + dot(vec4(-0.120399, 0.111831, 0.026343, -0.102694), i8),
                dot(vec4(0.102910, 0.008908, 0.044348, 0.062449), i0)
                + dot(vec4(-0.126696, -0.064937, -0.180247, -0.112804), i1)
                + dot(vec4(-0.115243, -0.193567, 0.043606, -0.124833), i2)
                + dot(vec4(-0.062924, -0.133156, -0.064626, 0.084112), i3)
                + dot(vec4(0.030771, 0.002298, -0.000656, 0.089425), i4)
                + dot(vec4(-0.161515, 0.055494, 0.041685, 0.051028), i5)
                + dot(vec4(0.019412, -0.219032, -0.184991, -0.067512), i6)
                + dot(vec4(-0.127191, -0.045591, -0.153685, -0.084036), i7)
                + dot(vec4(0.103370, 0.043078, 0.032894, 0.049990), i8),
                dot(vec4(0.065672, -0.101070, 0.161945, 0.213153), i0)
                + dot(vec4(-0.026870, 0.144608, 0.116851, 0.078037), i1)
                + dot(vec4(0.094212, -0.002451, -0.164057, 0.022745), i2)
                + dot(vec4(0.206397, -0.007944, 0.042638, 0.047044), i3)
                + dot(vec4(-0.032664, -0.173766, 0.291211, -0.445437), i4)
                + dot(vec4(0.045503, 0.054940, -0.097214, -0.179566), i5)
                + dot(vec4(-0.029214, 0.005941, 0.053188, 0.196282), i6)
                + dot(vec4(0.118253, -0.024858, -0.226870, 0.110927), i7)
                + dot(vec4(0.061612, 0.095644, 0.049958, -0.273031), i8),
                dot(vec4(-0.033201, 0.156419, 0.096741, -0.041702), i0)
                + dot(vec4(0.181530, 0.004518, -0.320916, -0.218269), i1)
                + dot(vec4(0.221304, 0.118310, 0.255394, -0.074993), i2)
                + dot(vec4(0.200092, -0.419673, 0.134842, -0.124535), i3)
                + dot(vec4(0.161651, 0.097075, 0.644595, 0.173146), i4)
                + dot(vec4(-0.159431, 0.286670, -0.768345, 0.016023), i5)
                + dot(vec4(0.060696, -0.134226, 0.140689, 0.046302), i6)
                + dot(vec4(-0.245726, -0.171657, 0.307320, 0.142357), i7)
                + dot(vec4(-0.066769, 0.020350, -0.257485, -0.115686), i8)
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
                dot(vec4(-0.018325, 0.149648, -0.082402, 0.033964), i0)
                + dot(vec4(0.023782, 0.229407, -0.045864, -0.088932), i1)
                + dot(vec4(-0.021418, 0.255098, -0.056806, 0.153006), i2)
                + dot(vec4(-0.106640, 0.348387, -0.227051, 0.096621), i3)
                + dot(vec4(0.023017, 0.298655, 0.025602, 0.014380), i4)
                + dot(vec4(-0.001324, 0.545215, 0.094708, 0.177678), i5)
                + dot(vec4(-0.187087, 0.345223, 0.062875, 0.071382), i6)
                + dot(vec4(0.002408, -0.009074, -0.037197, 0.035736), i7)
                + dot(vec4(-0.217074, 0.068392, 0.321301, 0.084296), i8),
                dot(vec4(-0.121149, 0.066057, -0.071395, -0.040514), i0)
                + dot(vec4(0.029804, -0.163913, 0.101760, -0.047718), i1)
                + dot(vec4(0.042148, -0.168790, 0.019963, -0.161576), i2)
                + dot(vec4(-0.059593, -0.179683, 0.108254, -0.101777), i3)
                + dot(vec4(-0.061786, 0.049698, -0.222919, 0.129823), i4)
                + dot(vec4(0.042271, -0.080207, -0.183387, 0.031769), i5)
                + dot(vec4(-0.007091, 0.033075, -0.187115, -0.035809), i6)
                + dot(vec4(-0.034558, 0.002412, 0.048261, 0.071338), i7)
                + dot(vec4(-0.178788, -0.123313, -0.093770, 0.084784), i8),
                dot(vec4(0.036287, -0.052824, 0.045599, -0.117738), i0)
                + dot(vec4(0.376734, 0.099210, 0.014417, -0.082741), i1)
                + dot(vec4(0.034357, 0.215518, -0.126648, 0.081939), i2)
                + dot(vec4(0.083013, 0.107371, 0.315375, 0.021730), i3)
                + dot(vec4(0.050645, 0.301662, -0.038498, -0.170018), i4)
                + dot(vec4(-0.025551, 0.393907, 0.117172, 0.145146), i5)
                + dot(vec4(-0.243432, 0.399352, -0.075683, -0.017114), i6)
                + dot(vec4(-0.071974, 0.167047, 0.118593, -0.179829), i7)
                + dot(vec4(0.008731, 0.207433, -0.300596, 0.048988), i8),
                dot(vec4(0.255343, 0.680235, -0.286143, -0.093889), i0)
                + dot(vec4(-0.127126, 0.503641, -0.388080, -0.024064), i1)
                + dot(vec4(-0.253498, 0.904208, 0.273362, -0.035296), i2)
                + dot(vec4(-0.112993, 0.383659, -0.157620, -0.138079), i3)
                + dot(vec4(0.195409, -0.053022, 0.034872, 0.137277), i4)
                + dot(vec4(0.135151, 0.415028, 0.235549, -0.097270), i5)
                + dot(vec4(-0.303193, 0.590095, 0.158434, -0.094003), i6)
                + dot(vec4(-0.115856, 0.104125, -0.126270, -0.061256), i7)
                + dot(vec4(0.204356, 0.076934, 0.253068, -0.010280), i8)
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
                dot(vec4(0.040069, -0.015694, -0.004980, -0.009770), i0)
                + dot(vec4(-0.144292, -0.028570, -0.015987, 0.110953), i1)
                + dot(vec4(0.053188, 0.007992, -0.042737, 0.193438), i2)
                + dot(vec4(-0.007410, -0.007965, -0.079018, 0.162108), i3)
                + dot(vec4(0.200556, -0.148639, 0.046298, 0.124175), i4)
                + dot(vec4(0.065371, -0.002078, 0.019155, 0.347527), i5)
                + dot(vec4(0.002833, -0.061902, 0.004744, 0.129310), i6)
                + dot(vec4(-0.074130, 0.093815, 0.040599, 0.150440), i7)
                + dot(vec4(0.028989, -0.047790, -0.027300, 0.072952), i8),
                dot(vec4(0.110524, -0.069367, -0.077912, -0.028848), i0)
                + dot(vec4(0.007441, -0.175914, 0.093567, -0.174093), i1)
                + dot(vec4(0.141694, 0.028383, -0.251283, -0.026419), i2)
                + dot(vec4(0.180334, -0.139906, 0.113216, -0.047568), i3)
                + dot(vec4(-0.021458, -0.025174, -0.070227, -0.038731), i4)
                + dot(vec4(0.110844, -0.186830, -0.219435, -0.133346), i5)
                + dot(vec4(-0.121881, 0.094391, 0.066150, -0.064653), i6)
                + dot(vec4(0.045222, 0.015989, -0.107503, 0.111807), i7)
                + dot(vec4(0.103703, -0.120356, 0.044425, -0.176199), i8),
                dot(vec4(0.248764, 0.018714, -0.058719, 0.007238), i0)
                + dot(vec4(-0.068812, -0.010544, -0.081968, -0.006695), i1)
                + dot(vec4(-0.007712, -0.056213, -0.058822, 0.124890), i2)
                + dot(vec4(0.190982, 0.050951, -0.152939, 0.096164), i3)
                + dot(vec4(-0.199673, -0.037904, 0.005410, 0.010185), i4)
                + dot(vec4(-0.019229, -0.120263, 0.035829, 0.263249), i5)
                + dot(vec4(-0.003393, 0.008079, -0.074079, 0.130535), i6)
                + dot(vec4(-0.284665, 0.153099, -0.006016, 0.295066), i7)
                + dot(vec4(-0.107790, 0.097993, -0.064108, 0.183704), i8),
                dot(vec4(-0.320889, -0.086486, -0.689302, 0.357810), i0)
                + dot(vec4(0.407111, -0.042077, -0.435056, 0.200238), i1)
                + dot(vec4(-0.065575, -0.300627, -0.583629, 0.397191), i2)
                + dot(vec4(0.182695, -0.200847, -0.281426, 0.400661), i3)
                + dot(vec4(-0.264409, 0.094309, -0.127652, 0.288219), i4)
                + dot(vec4(0.022582, -0.123209, -0.214768, 0.333030), i5)
                + dot(vec4(-0.096511, -0.179765, -0.523513, 0.430913), i6)
                + dot(vec4(-0.076188, 0.244653, -0.491110, 0.452480), i7)
                + dot(vec4(0.126343, -0.283292, -0.535768, 0.475463), i8)
            );
        
        gl_FragColor = sum + vec4(-0.016723, 0.000210, 0.098769, 0.026895);
    
        }
    )
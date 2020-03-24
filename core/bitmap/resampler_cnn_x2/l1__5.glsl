STRINGIFY(
        uniform sampler2D image;
        varying highp vec2 texCoord;
        uniform highp vec2 d1;
        uniform highp vec2 d2;

        lowp float fetch(float x, float y) {
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
        i[0] = vec4(fetch(x0, y0), fetch(x1, y0), fetch(x2, y0), fetch(x3, y0));
        i[1] = vec4(fetch(x4, y0), fetch(x0, y1), fetch(x1, y1), fetch(x2, y1));
        i[2] = vec4(fetch(x3, y1), fetch(x4, y1), fetch(x0, y2), fetch(x1, y2));
        i[3] = vec4(fetch(x2, y2), fetch(x3, y2), fetch(x4, y2), fetch(x0, y3));
        i[4] = vec4(fetch(x1, y3), fetch(x2, y3), fetch(x3, y3), fetch(x4, y3));
        i[5] = vec4(fetch(x0, y4), fetch(x1, y4), fetch(x2, y4), fetch(x3, y4));
        lowp float i6 = fetch(x4, y4);
    
        gl_FragColor = vec4(
            dot(i[0], vec4(0.069264, -0.214885, -0.052756, 0.030973)) + dot(i[1], vec4(-0.096295, -0.092459, -0.031359, 1.031047)) +
            dot(i[2], vec4(1.037557, -0.120791, -0.071716, 0.042203)) + dot(i[3], vec4(-0.490315, -0.569239, 0.027422, -0.013378)) +
            dot(i[4], vec4(0.208798, -0.384263, -0.367663, 0.080142)) + dot(i[5], vec4(-0.002693, 0.098235, -0.087663, -0.077110)) + i6 * 0.077453,
            dot(i[0], vec4(0.001329, 0.019670, 0.005405, -0.009687)) + dot(i[1], vec4(-0.015019, -0.006831, -0.028512, -0.102738)) +
            dot(i[2], vec4(0.124537, 0.015723, 0.001266, -0.073335)) + dot(i[3], vec4(-1.357595, 1.325234, 0.091238, 0.010662)) +
            dot(i[4], vec4(-0.029583, -0.088809, 0.092059, 0.018138)) + dot(i[5], vec4(-0.005871, 0.004655, 0.021085, -0.012982)) + i6 * -0.003515,
            dot(i[0], vec4(-0.054335, -0.058825, -0.046340, 0.121143)) + dot(i[1], vec4(-0.011529, 0.050646, 0.049626, -0.016628)) +
            dot(i[2], vec4(0.037715, -0.032142, -0.023663, 0.318248)) + dot(i[3], vec4(0.418798, -1.407882, -0.156168, 0.078901)) +
            dot(i[4], vec4(-0.239072, -0.235500, 1.148007, 0.004443)) + dot(i[5], vec4(-0.015152, -0.096576, -0.132279, 0.038594)) + i6 * 0.246321,
            dot(i[0], vec4(0.003785, 0.005175, -0.010711, 0.016290)) + dot(i[1], vec4(-0.014494, 0.009889, -0.018227, -0.001423)) +
            dot(i[2], vec4(0.104973, -0.006010, -0.008114, -0.030269)) + dot(i[3], vec4(0.109929, 1.303316, 0.122621, -0.001297)) +
            dot(i[4], vec4(0.028639, -0.088805, -1.290107, -0.102052)) + dot(i[5], vec4(0.013053, 0.019262, -0.022720, -0.128613)) + i6 * -0.026498
        ) + vec4(0.00428, -0.00118, -0.01285, -0.00032);
    
        }
    )
STRINGIFY(
beatmupInputImage image;
uniform sampler2D convnetOutput;
uniform highp vec2 size;
varying highp vec2 texCoord;

void main() {
    vec2 pos = mod(texCoord * size + vec2(0.5, 0.5), 2.0);
    vec4 yy = texture2D(convnetOutput, texCoord);
    
    float y;
    if (pos.y < 1.0)
        if (pos.x < 1.0)
            y = yy[0];
        else
            y = yy[1];
    else
        if (pos.x < 1.0)
            y = yy[2];
        else
            y = yy[3];

    vec3 i = texture2D(image, texCoord).rgb;
    float cb = -0.168736 * i.r - 0.331264 * i.g + 0.500000 * i.b;
    float cr = +0.500000 * i.r - 0.418688 * i.g - 0.081312 * i.b;

    gl_FragColor = vec4(
        y + 1.402 * cr,
        y - 0.344136 * cb - 0.714136 * cr,
        y + 1.772 * cb,
        1
    );
}
)

STRINGIFY(
beatmupInputImage image;
uniform sampler2D convnetOutput;
varying highp vec2 texCoord;

void main() {
    lowp vec4 yy = texture2D(convnetOutput, texCoord);
    highp vec2 pos = mod(gl_FragCoord.xy, 2.0);
    
    lowp float y;
    if (pos.y < 0.9)
        if (pos.x < 0.9)
            y = yy[0];
        else
            y = yy[1];
    else
        if (pos.x < 0.9)
            y = yy[2];
        else
            y = yy[3];

    mediump vec3 i = texture2D(image, texCoord).rgb;
    mediump float cb = -0.168736 * i.r - 0.331264 * i.g + 0.500000 * i.b;
    mediump float cr = +0.500000 * i.r - 0.418688 * i.g - 0.081312 * i.b;

    gl_FragColor = vec4(
        y + 1.402 * cr,
        y - 0.344136 * cb - 0.714136 * cr,
        y + 1.772 * cb,
        1.0
    );
}
)

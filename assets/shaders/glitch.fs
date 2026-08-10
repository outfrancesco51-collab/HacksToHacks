#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;

out vec4 finalColor;

// Pseudo-random generator
float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec2 uv = fragTexCoord;

    // Heavy Glitch Effect
    float glitchAmount = 0.03 * sin(time * 15.0);
    float noise = rand(vec2(floor(uv.y * 50.0), time));
    if (noise > 0.95) {
        uv.x += glitchAmount * sin(time * 50.0);
    }
    
    // Chromatic Aberration
    float offset = 0.005 * sin(time * 10.0);
    vec4 colR = texture(texture0, uv + vec2(offset, 0.0));
    vec4 colG = texture(texture0, uv);
    vec4 colB = texture(texture0, uv - vec2(offset, 0.0));
    
    vec4 color = vec4(colR.r, colG.g, colB.b, 1.0);
    
    // Scanlines
    float scanline = sin(uv.y * 800.0) * 0.04;
    color.rgb -= scanline;

    finalColor = color * colDiffuse * fragColor;
}

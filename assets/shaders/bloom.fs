#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

out vec4 finalColor;

void main()
{
    vec2 uv = fragTexCoord;
    vec4 sum = vec4(0.0);
    vec2 texelSize = vec2(1.0 / 1280.0, 1.0 / 720.0);
    
    // Simple 9-tap gaussian blur for bloom
    sum += texture(texture0, uv - 4.0 * texelSize) * 0.051;
    sum += texture(texture0, uv - 3.0 * texelSize) * 0.0918;
    sum += texture(texture0, uv - 2.0 * texelSize) * 0.12245;
    sum += texture(texture0, uv - 1.0 * texelSize) * 0.1531;
    sum += texture(texture0, uv) * 0.1633;
    sum += texture(texture0, uv + 1.0 * texelSize) * 0.1531;
    sum += texture(texture0, uv + 2.0 * texelSize) * 0.12245;
    sum += texture(texture0, uv + 3.0 * texelSize) * 0.0918;
    sum += texture(texture0, uv + 4.0 * texelSize) * 0.051;

    vec4 baseColor = texture(texture0, uv);
    
    // Additive blend with threshold
    vec4 bloom = max(sum - 0.5, 0.0) * 2.0;
    
    finalColor = (baseColor + bloom) * colDiffuse * fragColor;
}

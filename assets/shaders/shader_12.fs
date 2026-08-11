#version 100
precision mediump float;
varying vec2 fragTexCoord;
varying vec4 fragColor;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;

void main() {
    vec2 uv = fragTexCoord;
    vec4 texel = texture2D(texture0, uv);
    
    texel.b += cos(uv.x * 65.0 - time) * 0.1;
    
    gl_FragColor = texel * colDiffuse * fragColor;
}

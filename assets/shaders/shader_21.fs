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
    
    uv.x += sin(time * 2.2) * 0.01; texel = texture2D(texture0, uv);
    
    gl_FragColor = texel * colDiffuse * fragColor;
}

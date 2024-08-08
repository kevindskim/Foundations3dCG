#version 130

uniform float uVertexScale;  // Blending factor (0.0 to 1.0)
uniform sampler2D uTex0, uTex1;  // Texture samplers

in vec2 vTexCoord;  // Texture coordinates

void main(void) {
    // Sample colors from the two textures
    vec4 texColor0 = texture2D(uTex0, vTexCoord);
    vec4 texColor1 = texture2D(uTex1, vTexCoord);

    // Blend between texColor0 and texColor1
    // uVertexScale should be in the range [0.0, 1.0] to control blending
    float lerper = clamp(0.9*uVertexScale, 0.0, 1.0);
    vec4 blendedColor = mix(texColor0, texColor1, lerper);

    // Output the final color
    gl_FragColor = blendedColor;
}

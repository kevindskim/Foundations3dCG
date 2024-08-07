#version 130

uniform sampler2D uTex2;

in vec2 vTexCoord;
in vec3 vColor;

void main(void) {
  /* blend the vertex's color and the shield image */
  gl_FragColor = 0.5 * vec4(vColor.x, vColor.y, vColor.z, 1) + 0.5 * texture2D(uTex2, vTexCoord);
}

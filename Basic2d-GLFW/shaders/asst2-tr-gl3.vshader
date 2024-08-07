#version 130

uniform float uXCoefficient;
uniform float uYCoefficient;
uniform float uXOffset;
uniform float uYOffset;

in vec2 aPosition;
in vec2 aTexCoord;
in vec3 aColor;

out vec2 vTexCoord;
out vec3 vColor;

void main() {
  /* use the coefficients passed in as uniform variables to maintain the aspect ratio of the triangle */
  gl_Position = vec4((aPosition.x + uXOffset) * uXCoefficient, (aPosition.y + uYOffset) * uYCoefficient, 0, 1);

  vTexCoord = aTexCoord;
  vColor = aColor;
}

#version 130

attribute vec4 position;
attribute vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform vec4 eye_position;

varying vec3 vNormal;
varying vec3 vViewDirection;

void main(void)
{
  vec4 worldPosition = model * position;
  vViewDirection = (eye_position - normalize(worldPosition)).xyz;
  mat4 modelViewMatrix = view * model;
  vNormal = mat3(modelViewMatrix) * normal;

  gl_Position = projection * view * worldPosition;
}
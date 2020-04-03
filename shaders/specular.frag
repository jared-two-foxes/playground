#version 130
precision highp float;

varying vec3 vNormal;
varying vec3 vViewDirection;

vec3 scene_ambient = vec3(0.1,0.1,0.1);

struct LightSource
{
  vec4 position;
  vec3 diffuse;
};
LightSource light0 = LightSource(
    vec4(1.0, 1.0, 1.0, 0.0),
    vec3(1.0, 1.0, 1.0)
);

void main(void)
{
  vec3 lightVector;

  if ( light0.position.w == 0.0 ) {
    // Directional Light.
    // The light position is the direction is from which the light is
    // coming from.
    lightVector = vec3(light0.position);
  } 

  vec3 lightDirection  = normalize(lightVector);
  vec3 normalDirection = normalize(vNormal);
  vec3 viewDirection = normalize(vViewDirection);

  float diff = clamp(dot(lightDirection, normalDirection), 0.0, 1.0);

  vec3 ambientReflection = scene_ambient;

  vec3 diffuseReflection = 
     light0.diffuse * diff;

  float shadow = clamp(4*diff, 0, 1.0);

  vec3 Reflect = normalize(2 * diff * normalDirection - lightDirection);
  float specularReflection = pow(clamp(dot(Reflect, viewDirection), 0.0, 1.0), 8);

  // Pass varyings to fragment shader
  gl_FragColor = vec4(ambientReflection + shadow * ( diffuseReflection + specularReflection ), 1.0);
}
#version 430 core
in vec4 vPosition; 

out float intensity;
out float amplitude; 

uniform mat4 mvp;
void main() { 
  intensity = vPosition.w;
  vec4 t = vPosition;
  t.w = 1;
  gl_Position = t * mvp;
  float amp = 0.3 / ( gl_Position.w * gl_Position.w );
  float ps = floor( sqrt( amp ) ) + 1;
  gl_PointSize = ps;
  amplitude = amp * 0.7 / ( ps * ps );
}


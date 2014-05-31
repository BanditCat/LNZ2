#version 430 core
layout( location = 0 ) out vec4 color;

in float intensity;
in float amplitude;

void main(void) { 
  if( intensity < ( 1.0 / 5.0 ) )
    color = mix(vec4(0.01f, 0.01f, 0.01f, 1.0f), 
		vec4(1.0f, 0.01f, 0.01f, 1.0f), 
		(intensity * 5.0));
  else if( intensity < ( 2.0 / 5.0 ) )
    color = mix(vec4(1.0f, 0.01f, 0.01f, 1.0f), 
		vec4(1.0f, 0.01f, 0.5f, 0.5f), 
		((intensity - 1.0 / 5.0) * 5));
  else if( intensity < ( 3.0 / 5.0 ) )
    color = mix(vec4(1.0f, 0.01f, 0.5f, 0.5f), 
		vec4(0.01f, 0.01f, 1.0f, 1.0f), 
		((intensity - 2.0 / 5.0) * 5));
  else if( intensity < ( 4.0 / 5.0 ) )
    color = mix(vec4(0.01f, 0.01f, 1.0f, 1.0f), 
		vec4(0.01f, 0.5f, 0.5f, 1.0f), 
		((intensity - 3.0 / 5.0) * 5));
  else if( intensity < ( 5.0 / 5.0 ) )
    color = mix(vec4(0.01f, 0.5f, 0.5f, 1.0f), 
		vec4(0.01f, 1.0f, 0.01f, 1.0f), 
		((intensity - 4.0 / 5.0) * 5));
  color *= ( intensity * 0.2 + 0.3 );
  color *= amplitude;
}


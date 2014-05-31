#version 430 core
// Uniform block containing positions and masses of the attractors 

layout (std140, binding = 0) uniform attractor_block { 
  vec4 attractor[64]; // xyz = position, w = mass 
};

// Process particles in blocks of 32 
layout (local_size_x = 1024) in;

// Buffers containing the positions and velocities of the particles 
layout (rgba32f, binding = 0) uniform imageBuffer velocity_buffer; 
layout (rgba32f, binding = 1) uniform imageBuffer position_buffer;
layout (rgba32f, binding = 2) uniform imageBuffer velocity_buffer_out; 
layout (rgba32f, binding = 3) uniform imageBuffer position_buffer_out;

// Delta time 
uniform float dt;

void main(void) { 
  // Read the current position and velocity from the buffers 
  vec4 vel = imageLoad(velocity_buffer, int(gl_GlobalInvocationID.x)); 
  vec4 pos = imageLoad(position_buffer, int(gl_GlobalInvocationID.x));
  int i,c;
  // Update position using current velocity * time 
  pos.xyz += vel.xyz * dt; 
  // Update "life" of particle in w component
  pos.w -= 0.004 * dt;
  // Loop for hi res
  for( c = 0; c < 2; ++c ){
    // For each attractor... 
    for (i = 0; i < 64; i++) { 
      // Calculate force and update velocity accordingly 
      vec3 dist = (attractor[i].xyz - pos.xyz) * 0.7; 
      vec3 accl = dt * 0.5 * 0.075 * attractor[i].w * 
    	normalize(dist) / (dot(dist, dist));
      //      accl = clamp( accl, vec3( -0.7 ), vec3( 0.7 ) );
      vel.xyz += accl;
    }
  }
  //if( length( vel.xyz ) > 1.5 )
    //  vel.xyz = normalize(vel.xyz) * 1. 5;
  // If the particle expires, reset it 
  if (pos.w <= 0.0) { 
    pos.xyz = -pos.xyz * 0.01; 
    vel.xyz *= 0.01;
    pos.w += 1.0f; 
  }
  // Store the new position and velocity back into the buffers
  imageStore(position_buffer_out, int(gl_GlobalInvocationID.x), pos);
  imageStore(velocity_buffer_out, int(gl_GlobalInvocationID.x), vel);

}

////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#version 430 core

layout( std140, binding = 0 ) uniform attractor_block{ 
  vec4 attractor[64]; // w is mass 
};

layout( local_size_x = 1024 ) in;

layout( rgba32f, binding = 0 ) uniform imageBuffer vbuffer; 
layout( rgba32f, binding = 1 ) uniform imageBuffer pbuffer;
layout( rgba32f, binding = 2 ) uniform imageBuffer vbuffer_out; 
layout( rgba32f, binding = 3 ) uniform imageBuffer pbuffer_out;

layout( std140, binding = 5 ) buffer ssbo{
  uvec4 gb[];
};

// Time change per frame in seconds.
uniform float dt;
// Viewport size.
uniform vec4 screen;
// Modelview Projection Matrix.
uniform mat4 mvp;

void main( void ){ 
  vec4 vel = imageLoad( vbuffer, int( gl_GlobalInvocationID.x ) ); 
  vec4 pos = imageLoad( pbuffer, int( gl_GlobalInvocationID.x ) );
  int i,c;

  pos.xyz += vel.xyz * dt; 

  pos.w -= 0.004 * dt;

  // Loop for hi res
  //  for( c = 0; c < 2; ++c ){
    for( i = 0; i < 64; i++ ){ 
      vec3 dist = ( attractor[ i ].xyz - pos.xyz ) * 0.7; 
      vec3 accl = dt * 0.075 * attractor[ i ].w * 
    	normalize(dist) / dot( dist, dist );
      //      accl = clamp( accl, vec3( -0.7 ), vec3( 0.7 ) );
      vel.xyz += accl;
    }
    //  }
  //if( length( vel.xyz ) > 1.5 )
    //  vel.xyz = normalize(vel.xyz) * 1. 5;
  // If the particle expires, reset it 
  if( pos.w <= 0.0 ){ 
    pos.w += 1.0f; 
    vel.xyz *= 0.01;
    pos.xyz = -pos.xyz * 0.01; 
  }

  imageStore( pbuffer_out, int( gl_GlobalInvocationID.x ), pos );
  imageStore( vbuffer_out, int( gl_GlobalInvocationID.x ), vel );

  {
    vec4 spos = pos;
    spos.w = 1;
    spos = ( spos * mvp );
    spos /= spos.w;
    spos.xy *= 0.5;
    spos.xy += 0.5;
    
    if( spos.z > 0.0 && spos.z < 1 &&
	spos.x >= 0 && spos.x < 1 &&
	spos.y >= 0 && spos.y < 1 ){
      // BUGBUG need to scale by pixel size.
      float amp = spos.z * spos.z * 10;
      float r = sqrt( 3 * amp / 3.141592653589793238462643383 );
      int xs = int( spos.x * screen.x - r );
      if( xs < 0 )
	xs = 0;
      int xe = int( spos.x * screen.x + r );
      if( xe >= screen.x )
	xe = int( screen.x ) - 1;
      int ys = int( spos.y * screen.y - r );
      if( ys < 0 )
	ys = 0;
      int ye = int( spos.y * screen.y + r );
      if( ye >= screen.y )
	ye = int( screen.y ) - 1;
      float tot = 0;
      //      xs = xe = int( xs + r ); ys = ye = int( ys + r );
      for( int x = xs; x <= xe; ++x ){
	for( int y = ys; y <= ye; ++y ){
	  vec2 dst = { x, y };
	  dst /= screen.xy;
	  dst -= spos.xy;
	  dst *= screen.xy;
	  tot += clamp( 1 - sqrt( dot( dst.xy, dst.xy ) ) / r, 0, 1 );
	}
      }
      for( int x = xs; x <= xe; ++x ){
	for( int y = ys; y <= ye; ++y ){
	  vec2 dst = { x, y };
	  dst /= screen.xy;
	  dst -= spos.xy;
	  dst *= screen.xy;
	  float b = amp
	    * ( clamp( 1 - sqrt( dot( dst.xy, dst.xy ) ) / r, 0, 1 ) / tot );
	  //	    imageLoad( gbuffer, x + y * int( screen.x ) ).y; 
	  //	  imageStore( gbuffer, x + y * int( screen.x ), 
	  //	      vec4( 0, v, 0, 0 ) );
	  int ax = x / 2;
	  int ay = y / 2;
	  int sel = ( y % 2 ) * 2 + x % 2;
	  uint v;
	  switch( sel ){
	  case 0:
	    v = gb[ ax + ay * int( screen.x ) ].x;
	    break;
	  case 1:
	    v = gb[ ax + ay * int( screen.x ) ].y;
	    break;
	  case 2:
	    v = gb[ ax + ay * int( screen.x ) ].z;
	    break;
	  case 3:
	    v = gb[ ax + ay * int( screen.x ) ].w;
	    break;
	  }
	  uint hcount = v >> 22;
	  uint hue = ( v >> 12 ) & 1023;
	  uint intensity = v & 4095;	

	  //if( intensity + int( amp * 1000 ) < 4095 )
	  uint dif = int( round( b * 4095 ) );
	  if( intensity + dif < 4095 )
	    intensity += dif;
	  else
	    intensity = 4095;
  
	  if( hcount < 1023 ){
	    hcount += 1;
	    float nhue = float( hue ) * 
	      ( float( hcount - 1 ) / float( hcount ) ) +
	      clamp( pos.w * 1023, 0, 1023 ) / float( hcount );
	    hue = int( round( clamp( nhue, 0, 1023 ) ) );
	  }

	  v = ( hcount << 22 ) + ( hue << 12 ) + intensity;

	  switch( sel ){
	  case 0:
	    gb[ ax + ay * int( screen.x ) ].x = v;
	    break;
	  case 1:
	    gb[ ax + ay * int( screen.x ) ].y = v;
	    break;
	  case 2:
	    gb[ ax + ay * int( screen.x ) ].z = v;
	    break;
	  case 3:
	    gb[ ax + ay * int( screen.x ) ].w = v;
	    break;
	  }

	  //	  gb[ x + y * int( screen.x ) ].w += 1;
	}
      }
    }      
  }

}


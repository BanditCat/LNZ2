////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Jonathan(Jon) DuBois 2014. This file is part of LNZ.         //
////////////////////////////////////////////////////////////////////////////////

#version 430 core
layout( location = 0 ) out vec4 color;

layout( r32ui, binding = 4 ) uniform uimageBuffer gbuffer;

layout( std140, binding = 5 ) coherent buffer ssbo{
  uvec4 gb[];
};

uniform vec4 screen;

void main(void) { 
  vec4 ans;
  int ax = int( gl_FragCoord.x ) / 2;
  int ay = int( gl_FragCoord.y ) / 2;
  int sel = ( int( gl_FragCoord.y ) % 2 ) * 2 + int( gl_FragCoord.x ) % 2;
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
  uint hue = ( v >> 12 ) & 1023;
  hue = hue + 512;
  if( hue > 1023 )
    hue -= 1024;
  uint intensity = v & 4095;

  float val = clamp( intensity * 0.00025, 0, 1 );
  float sat = 0.5;

  float h = ( hue * 6 / 1024 );
  int hs = int( floor( h ) );
  h -= hs;
  float p = val * ( 1 - sat );
  float q = val * ( 1 - sat * h );
  float t = val * ( 1 - sat * ( 1 - h ) );
  switch( hs ){
  case 0:
    ans.r = val;
    ans.g = t;
    ans.b = p;
    break;
  case 1:
    ans.r = q;
    ans.g = val;
    ans.b = p;
    break;
  case 2:
    ans.r = p;
    ans.g = val;
    ans.b = t;
    break;
  case 3:
    ans.r = p;
    ans.g = q;
    ans.b = val;
    break;
  case 4:
    ans.r = t;
    ans.g = p;
    ans.b = val;
    break;
  case 5:
    ans.r = val;
    ans.g = p;
    ans.b = q;
    break;
  }
  ans.a = 1;

  color = ans;
}


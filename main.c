// Main entry point.
#include "lnz.h"
#include <stdio.h>
#include "math.h"

#define PARTICLE_GROUPS 4096
#define PARTICLE_COUNT ( 1024 * PARTICLE_GROUPS )

int dwidth, dheight;
float sfps = 30.0;
float rotx = 0, roty = 0, drotx = 0, droty = 0;//`0.05, droty = 0.02;
int rel = 0;
int blowup = 0;
float scale = 1;
lvec trns = { 0, 0, 90 };
u64 disableMouseTime = 0;

void keys( const SDL_Event* ev ){ 
  if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_ESCAPE )
    exit( EXIT_SUCCESS );
  else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_f )
    SDL_Log( "\nfps: %.3f", sfps );
  else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_r ){
    if( rel ){
      rel = 0;
      drotx = droty = 0;
    } else
      rel = 1;
  } else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_b ){
    if( blowup )
      blowup = 0;
    else
      blowup = 1;
  } else if( ev->key.state == SDL_PRESSED && ev->key.keysym.sym == SDLK_SPACE ){
    if( scale )
      scale = 0;
    else
      scale = 1;
  }
}

void touches( const SDL_Event* ev ){
  disableMouseTime = SDL_GetPerformanceCounter();
  if( ev->tfinger.type == SDL_FINGERDOWN && ev->tfinger.x > 0.9 &&
      ev->tfinger.y < 0.1 &&
      SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 1 )
    exit( EXIT_SUCCESS );
  else if( ev->tfinger.type == SDL_FINGERDOWN && ev->tfinger.x < 0.1 &&
	   ev->tfinger.y < 0.1 && 
	   SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 1 ){
    if( rel ){
      rel = 0;
      drotx = droty = 0;
    } else
      rel = 1;
  } else if( ev->tfinger.type == SDL_FINGERDOWN && ev->tfinger.x > 0.9 &&
	   ev->tfinger.y > 0.9 && 
	   SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 1 ){
    if( blowup )
      blowup = 0;
    else
      blowup = 1;
  } else if( ev->tfinger.type == SDL_FINGERDOWN && ev->tfinger.x < 0.1 &&
	   ev->tfinger.y > 0.9 && 
	   SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 1 ){
    if( scale == 0.0 )
      scale = 1.0;
    else
      scale = 0.0;      
  } else{
    if( SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 1 ){
      if( rel ){
	drotx += ev->tfinger.dx;
	droty += ev->tfinger.dy;
      } else{
	rotx += ev->tfinger.dx;
	roty += ev->tfinger.dy;
      }
    } else if( SDL_GetNumTouchFingers( ev->tfinger.touchId ) == 2 ){
      SDL_Finger* f0 = SDL_GetTouchFinger( ev->tfinger.touchId, 0 );
      SDL_Finger* f1 = SDL_GetTouchFinger( ev->tfinger.touchId, 1 );
      float odx, ody, dx, dy;
      if( ev->tfinger.fingerId == f0->id ){
	dx = f0->x - f1->x;
	dy = f0->y - f1->y;
      }	else{
	dx = f1->x - f0->x;
	dy = f1->y - f0->y;
      }
      odx = dx - ev->tfinger.dx;
      ody = dy - ev->tfinger.dy;
      float odist = sqrt( pow( odx, 2.0 ) + pow( ody, 2.0 ) );
      float dist = sqrt( pow( dx, 2.0 ) + pow( dy, 2.0 ) );
      trns[ 2 ] += ( dist - odist ) * -400;
      trns[ 0 ] += ev->tfinger.dx * 100;
      trns[ 1 ] += ev->tfinger.dy * -100;
    }
  }
}

void wms( const SDL_Event* ev ){
  if( ev->window.event == SDL_WINDOWEVENT_RESIZED ){
    dwidth = ev->window.data1;
    dheight = ev->window.data2;
    glViewport( 0, 0, dwidth, dheight );
  }
}
void mice( const SDL_Event* ev ){
  if( ev->type == SDL_MOUSEMOTION && 
      ( SDL_GetPerformanceCounter() - disableMouseTime ) /
      (double)SDL_GetPerformanceFrequency() > 0.1 ){
    if( ev->motion.state & SDL_BUTTON_LMASK ){
      trns[ 0 ] += ev->motion.xrel * 0.05;
      trns[ 1 ] += ev->motion.yrel * -0.05;
    } else if( ev->motion.state & SDL_BUTTON_RMASK ){
      trns[ 2 ] += ev->motion.yrel * 0.2;
    }else if( rel ){
      drotx += ev->motion.xrel * 0.0005;
      droty += ev->motion.yrel * 0.0005;
    } else{
      rotx += ev->motion.xrel * 0.0005;
      roty += ev->motion.yrel * 0.0005;
    }
  }
}
    

int main( int argc, char* argv[] ){
  (void)(argc);(void)(argv);

  LNZInit( 1, "LNZ2.0a", 0.9, 0.675 );
  SDL_GL_GetDrawableSize( mainWindow, &dwidth, &dheight );
  SDL_GL_SetSwapInterval( 0 );
  srand( 1337 );

  LNZSetKeyHandler( keys );
  LNZSetTouchHandler( touches );
  LNZSetMouseHandler( mice );
  LNZSetWindowHandler( wms );

  u64 sz;
  u8* dt = LNZLoadResourceOrDie( "main.glsl", &sz );
  GLuint shd[ 2 ];
  shd[ 0 ] = LNZCompileOrDie( (char*)dt, GL_COMPUTE_SHADER );
  GLuint prg = LNZLinkOrDie( 1, shd );
  dt = LNZLoadResourceOrDie( "frag.frag", &sz );
  shd[ 0 ] = LNZCompileOrDie( (char*)dt, GL_FRAGMENT_SHADER );
  dt = LNZLoadResourceOrDie( "vert.vert", &sz );
  shd[ 1 ] = LNZCompileOrDie( (char*)dt, GL_VERTEX_SHADER );
  GLuint fprg = LNZLinkOrDie( 2, shd );




  // Generate two buffers, bind them and initialize their data stores
  GLuint buffers[ 4 ], texs[ 4 ];
  glGenBuffers( 4, buffers );

  glBindBuffer(GL_ARRAY_BUFFER, buffers[ 0 ] );
  glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof( GLfloat ) * 4,
	       NULL, GL_DYNAMIC_COPY );
  // Map the position buffer and fill it with random vectors
  GLfloat* positions = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );

  for( u32 i = 0; i < PARTICLE_COUNT; ++i ){
    for( u32 k = 0; k < 3; ++k )
      positions[ i * 4 + k ] = ( rand() / (double)RAND_MAX ) * 20.0 - 10.0;
    positions[ i * 4 + 3 ] = rand() / (double)RAND_MAX;
  }
  glUnmapBuffer( GL_ARRAY_BUFFER );

  glBindBuffer(GL_ARRAY_BUFFER, buffers[ 1 ] );
  glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof( GLfloat ) * 4,
	       NULL, GL_DYNAMIC_COPY );
  // Map the position buffer and fill it with random vectors
  GLfloat* velocities = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );

  for( u32 i = 0; i < PARTICLE_COUNT; ++i ){
    for( u32 k = 0; k < 3; ++k )
      velocities[ i * 4 + k ] = ( rand() / (double)RAND_MAX ) * 0.2 - 0.1;
    velocities[ i * 4 + 3 ] = 0;
  }
  glUnmapBuffer( GL_ARRAY_BUFFER );
  for( u32 i = 2; i < 4; ++i ){
    glBindBuffer( GL_ARRAY_BUFFER, buffers[ i ] );
    glBufferData( GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof( GLfloat ) * 4,
		  NULL, GL_DYNAMIC_COPY );
  }

  glGenTextures( 4, texs );
  for( u32 i = 0; i < 4; ++i ){
    glBindTexture( GL_TEXTURE_BUFFER, texs[ i ] );
    glTexBuffer( GL_TEXTURE_BUFFER, GL_RGBA32F, buffers[ i ] );
  }

  GLuint ubuf;
  glGenBuffers( 1, &ubuf );
  glBindBuffer( GL_UNIFORM_BUFFER, ubuf );
  glBufferData( GL_UNIFORM_BUFFER, 64 * sizeof( GLfloat ) * 4, NULL,
		GL_DYNAMIC_COPY );
  

  GLuint vao[ 2 ]; 
  glGenVertexArrays( 2, vao );
  glBindVertexArray( vao[ 1 ] );
  glBindBuffer( GL_ARRAY_BUFFER, buffers[ 2 ] );
  glBindAttribLocation( fprg, 0, "vPosition" );
  glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );
  glEnableVertexAttribArray( 0 );
  glBindVertexArray( vao[ 0 ] );
  glBindBuffer( GL_ARRAY_BUFFER, buffers[ 0 ] );
  glBindAttribLocation( fprg, 0, "vPosition" );
  glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0 );
  glEnableVertexAttribArray( 0 );
  glBindVertexArray( 0 );
  
  glEnable( GL_PROGRAM_POINT_SIZE );

  double time = 0;
  u64 ntime = SDL_GetPerformanceCounter();
  GLuint dtloc = glGetUniformLocation( prg, "dt" );
  GLuint mvploc = glGetUniformLocation( fprg, "mvp" );
  GLfloat amasses[ 64 ];
  int bsel = 0, nbsel = 2;

  for( u32 i = 0; i < 64; ++i )
    amasses[ i ] = rand() / (double)RAND_MAX + 1;
  while( 1 ){
    //    double b = rand() / (double)RAND_MAX;
    LNZLoop();
    if( glGetError() != GL_NO_ERROR )
      exit( 42 );
    u64 ttime = ntime;
    ntime = SDL_GetPerformanceCounter();
    float dtime = ( ntime - ttime ) / (double)SDL_GetPerformanceFrequency();
    float udtime = dtime;
    dtime *= (double)scale;
    time += 0.001 * dtime;
    if( dtime )
      sfps = sfps * 0.95 + 0.05 / dtime;
    
    glBindBuffer( GL_UNIFORM_BUFFER, ubuf );
    GLfloat* attractors =
      (GLfloat*)glMapBuffer( GL_UNIFORM_BUFFER, GL_WRITE_ONLY );

    for( int i = 0; i < 64; i++ ){
      attractors[ i * 4 + 0 ] =  sinf(time * (float)(i + 4) * 7.5f * 2.0f)
      	* 50.0f;
      attractors[ i * 4 + 1 ] = cosf(time * (float)(i + 7) * 3.9f * 2.0f)
      	* 50.0f;
      attractors[ i * 4 + 2 ] = sinf(time * (float)(i + 3) * 5.3f * 2.0f)
      	* cosf(time * (float)(i + 5) * 9.1f) * 100.0f;
      attractors[ i * 4 + 3 ] = amasses[ i ] * 15.0;
    }
    if( blowup ){
      attractors[ 0 ] = attractors[ 1 ] = attractors[ 2 ] = 0.0;
      attractors[ 1 ] = 10;
      attractors[ 3 ] = 16.15;
    }
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBufferBase( GL_UNIFORM_BUFFER, 0, ubuf );

    // Activate the compute program and bind the position
    // and velocity buffers
    glUseProgram(prg);

    glBindImageTexture(0, texs[ 1 + bsel ], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, texs[ 0 + bsel ], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(2, texs[ 1 + nbsel ], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(3, texs[ 0 + nbsel ], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    // Set delta time
    
    glUniform1f(dtloc, dtime * 20);
    // Dispatch the compute shader
    if( scale )
      glDispatchCompute(PARTICLE_GROUPS, 1, 1);
    // Ensure that writes by the compute shader have completed
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    // Set up our mvp matrix for viewing
   
    // Clear, select the rendering program and draw a full screen quad
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(fprg);
    {
      lmat mvp;
      rotx += udtime * drotx * 2;
      roty += udtime * droty * 2;
      // lmprojection( mvp, 90 )
      float aspect = sqrt( dwidth / (double)dheight );
      lvec sc = { 0.013 / aspect, 0.013 * aspect, 0.13 };
      lvec up = { cosf( rotx * 5 + pi / 2 ) * sinf( roty * 5 ), cosf( roty * 5 ), sinf( rotx * 5 + pi / 2 ) * sinf( roty * 5 ) };
      lvec right = { cosf( rotx * 5 ), 0.0, sinf( rotx * 5 ) };

      lmidentity( mvp );
      lmbasis( mvp, up, right );
      lmtranslate( mvp, trns );
      lmprojection( mvp, 0.0125 );
      lmscale( mvp, sc );
      glUniformMatrix4fv(mvploc, 1, GL_FALSE, mvp);
    }
    glBindVertexArray( vao[ bsel / 2 ] );
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
    SDL_GL_SwapWindow( mainWindow );
    if( bsel ){
      bsel = 0;
      nbsel = 2;
    } else{
      bsel = 2;
      nbsel = 0;
    }
  }
  exit( EXIT_SUCCESS );
}


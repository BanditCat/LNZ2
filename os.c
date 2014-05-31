// OS dependant code goes in this file.
#define WIN32_LEAN_AND_MEAN

#include "lnz.h"

#include <windows.h>
#include <SDL_syswm.h>


// BUGBUG starting the app does not close the soft keyboard. No known fix.
//WINUSERAPI WINBOOL WINAPI RegisterTouchWindow( HWND hWnd,ULONG ulFlags);
void LNZOSReinit( void ){
  /* struct SDL_SysWMinfo wi; */
  /* SDL_VERSION( &wi.version ); */
  
  /* if( SDL_GetWindowWMInfo( mainWindow, &wi ) == (u32)-1 ){ */
  /*   SDL_LogError( SDL_LOG_CATEGORY_APPLICATION, */
  /* 		  "\nFailed to get window system info.\n" ); */
  /*     exit( EXIT_FAILURE ); */
  /* } */
  /* RegisterTouchWindow( wi.info.win.window, 3 ); */
  /* SDL_RaiseWindow( mainWindow ); */
  /* SDL_UpdateWindowSurface( mainWindow ); */
  /* SetFocus( wi.info.win.window ); */
}
u8* LNZLoadResource( const char* name, u64* size ){
  HRSRC rsc;
  HGLOBAL hnd;
  u8* dt,* ans;
  if( ( rsc = FindResource( NULL, name, RT_RCDATA ) ) == NULL )
    return NULL;
  if( ( hnd = LoadResource( NULL, rsc ) ) == NULL )
    return NULL;
  if( ( dt = LockResource( hnd ) ) == NULL )
    return NULL;
  *size = SizeofResource( NULL, rsc );
  ans = malloc( *size + 1 );
  memcpy( ans, dt, *size );
  ans[ *size ] = '\0';
  return ans;
}

u8* LNZLoadResourceOrDie( const char* name, u64* size ){
  u8* ans = LNZLoadResource( name, size );
  if( ans == NULL ){
    SDL_LogError( SDL_LOG_CATEGORY_APPLICATION,
		  "\nFailed to load resource %s.\n",
		  name );
    exit( EXIT_FAILURE );
  }
  return ans;
}  

#ifndef LUACURL_H
#define LUACURL_H

#include "lua.h"
#include "lauxlib.h"

#ifdef WIN32
#include "../../curl/include/win32/curl/curl.h"
#include "../../curl/include/win32/curl/easy.h"

#ifndef LIBCURL_VERSION
  #include "../../curl/include/win32/curl/curlver.h"
#endif
#endif

#ifdef ANDROID
#include "../../curl/include/android/curl/curl.h"
#include "../../curl/include/android/curl/easy.h"

#ifndef LIBCURL_VERSION
  #include "../../curl/include/android/curl/curlver.h"
#endif
#endif

/*-------------------------------------------------------------------------*\
* This macro prefixes all exported API functions
\*-------------------------------------------------------------------------*/

#define LUACURL_API extern


/*-------------------------------------------------------------------------*\
* Initializes the library.
\*-------------------------------------------------------------------------*/
LUACURL_API int luaopen_luacurl (lua_State *L);

#endif
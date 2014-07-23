#ifdef _WIN32
#include <stdint.h>
#include <float.h>
#include <Windows.h>
#define INFINITY DBL_MAX
#define NAN DBL_MAX
#define HAVE_DECL_NAN
#define HAVE_DECL_INFINITY
#define HAVE_DECL_ISINF
#define isinf(x) (0) //不支持无穷大判断
#define isnan(x) (0)
#else
#include <stdint.h>
#include <math.h>
#include <string.h>

#define HAVE_DECL_NAN 1
#define HAVE_DECL_INFINITY 1
#define HAVE_DECL_ISINF 1
#define HAVE_SNPRINTF 1
#define HAVE_STRDUP 1
#define HAVE_STRNCASECMP 1
#define HAVE_VSNPRINTF 1
#define HAVE_VASPRINTF 1
#endif
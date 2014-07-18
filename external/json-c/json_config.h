#ifdef _WIN32
#include <stdint.h>
#include <float.h>
#define INFINITY DBL_MAX
#define HAVE_DECL_NAN
#define HAVE_DECL_INFINITY
#endif
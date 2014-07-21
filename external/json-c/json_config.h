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
#endif
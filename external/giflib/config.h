
// giflib config.h

#ifndef GIF_CONFIG_H_DEFINED
#define GIF_CONFIG_H_DEFINED

#include <sys/types.h>
#define HAVE_STDINT_H
#define HAVE_FCNTL_H
#ifdef __APPLE__
#define HAVE_UNISTED_H
#endif
#ifdef uint32_t
typedef uint32_t UINT32;
#else
typedef unsigned int UINT32;
#endif

#endif

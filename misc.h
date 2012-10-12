#ifndef _COMMON_H
#define _COMMON_H
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#define efatal(x) ffatal("%s: %s", x, strerror(errno))
#define effatal(x, ...) ffatal(x ": %s", ##__VA_ARGS__, strerror(errno))

#define fatal(x) ffatal("%s", x)

#define ffatal(x, ...) do { \
    debug(x, __VA_ARGS__); \
    _exit(1); \
} while(0)

#define debug(x, ...) fprintf(stderr, x"\n", ##__VA_ARGS__)
#endif

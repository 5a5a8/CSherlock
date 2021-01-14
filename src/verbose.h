#ifndef FILE_VERBOSE_SEEN
#define FILE_VERBOSE_SEEN

#include <stdbool.h>

int v_print(const char * restrict, ...);
void set_verbose(bool);

#endif

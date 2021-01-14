#include "verbose.h"
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

bool verbose_on = false;

void set_verbose(bool setting) {
	verbose_on = setting;
}

int v_print(const char * restrict format, ...){
	if (!verbose_on){
		return 0;
	}

	va_list args;
	va_start(args, format);
	int ret = vprintf(format, args);
	va_end(args);

	return ret;
}

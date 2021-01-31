/******************************************************************************/
/*    FILE: verbose.c                                                         */
/*    AUTHOR: 5a5a8 <5a5a8@protonmail.com>                                    */
/*    PURPOSE: Provide verbose printing and debug logging capability.         */
/*    LICENSE: GNU General Public Licence v3.0: see '../LICENCE' for more.    */
/******************************************************************************/

#include "verbose.h"
#include <time.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool verbose_on = false;
bool debug_on = false;

void set_verbose(bool setting) {
	verbose_on = setting;
}

void set_debug(bool setting){
	debug_on = setting;
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

int d_log(const char * restrict format, ...){
	/* This function writes to the debug log. It has effectively the same   */
	/* usage as printf, but it writes the current date-time-group to a file */
	/* along with the debug message.                                        */ 

	if (!debug_on){
		return 0;
	}

	pthread_mutex_lock(&debug_file_lock);
	FILE *fp = fopen(D_FILE, "a");

	/* Get the current UTC time for logging, and convert to string */
	time_t raw_time;
	struct tm * time_info;
	time(&raw_time);
	time_info = gmtime(&raw_time);

	char *dtg = malloc(1024 * sizeof(char));
	snprintf(dtg, 1024, "[%d-%d-%d %d:%d:%d UTC]: ",
			time_info->tm_year + 1900,
			time_info->tm_mon + 1,
			time_info->tm_mday,
			time_info->tm_hour,
			time_info->tm_min,
			time_info->tm_sec);

	/* Now we have [date time group] + format string in dtg. */
	strncat(dtg, format, 1024);

	/* Write the line to the file */
	va_list args;
	va_start(args, format);
	int ret = vfprintf(fp, dtg, args);
	va_end(args);

	free(dtg);
	fclose(fp);
	pthread_mutex_unlock(&debug_file_lock);

	return ret;
}

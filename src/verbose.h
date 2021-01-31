/******************************************************************************/
/*    FILE: verbose.h                                                         */
/*    AUTHOR: 5a5a8 <5a5a8@protonmail.com>                                    */
/*    PURPOSE: Provide verbose printing capability.                           */
/*    LICENSE: GNU General Public Licence v3.0: see '../LICENCE' for more.    */
/******************************************************************************/

#ifndef FILE_VERBOSE_SEEN
#define FILE_VERBOSE_SEEN

#include <pthread.h>
#include <stdbool.h>

#define D_FILE "debug.log"

pthread_mutex_t debug_file_lock;

void set_verbose(bool setting);
void set_debug(bool setting);
int v_print(const char * restrict format, ...);
int d_log(const char * restrict format, ...);

#endif

/******************************************************************************/
/*    FILE: verbose.h                                                         */
/*    AUTHOR: 5a5a8 <5a5a8@protonmail.com>                                    */
/*    PURPOSE: Provide verbose printing capability.                           */
/*    LICENSE: GNU General Public Licence v3.0: see '../LICENCE' for more.    */
/******************************************************************************/

#ifndef FILE_VERBOSE_SEEN
#define FILE_VERBOSE_SEEN

#include <stdbool.h>

int v_print(const char * restrict, ...);
void set_verbose(bool);

#endif

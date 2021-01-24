/******************************************************************************/
/*    FILE: regexcheck.h                                                      */
/*    AUTHOR: 5a5a8 <5a5a8@protonmail.com>                                    */
/*    PURPOSE: Provide regular expression matching with libpcre2.             */
/*    LICENSE: GNU General Public Licence v3.0: see '../LICENCE' for more.    */
/******************************************************************************/

#ifndef FILE_REGEXCHECK_SEEN
#define FILE_REGEXCHECK_SEEN

#include <stdbool.h>

#define PCRE2_CODE_UNIT_WIDTH 8

bool RE_check_regex(char *regex, char *username);

#endif

#ifndef FILE_REGEXCHECK_SEEN
#define FILE_REGEXCHECK_SEEN

#include <stdbool.h>

#define PCRE2_CODE_UNIT_WIDTH 8

bool RE_check_regex(char *regex, char *username);

#endif

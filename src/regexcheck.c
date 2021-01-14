#include "regexcheck.h"
#include <pcre2.h>
#include <stdbool.h>
#include <string.h>
#include "verbose.h"

bool RE_check_regex(char *regex, char *username){
	/**************************************************************************/
	/* regex      - PCRE compliant regular-expression to match                */
	/* username   - String to match the username against                      */
	/* return val - true if match, false otherwise                            */
	/**************************************************************************/

	/* Compile the regular expression */
	int errornumber;
	PCRE2_SIZE erroroffset;
	pcre2_code *re;

	re = pcre2_compile(
			(PCRE2_SPTR) regex,
			PCRE2_ZERO_TERMINATED,
			0,
			&errornumber,
			&erroroffset,
			NULL);

	/* re will be NULL if compilation failed */
	if (re == NULL){
		PCRE2_UCHAR buffer[256];
		pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
		v_print("WARNING: pcre2 failed to compile regex.\n");
		v_print("\tOffset: %d\n", (int)erroroffset);
		v_print("\tMessage: %s\n", buffer);
		return false;
	}

	/* This ensures that the match data block is exactly the right size for   */
	/* the number of capturing parentheses in the pattern. For the purpose of */
	/* CSherlock this is not really needed as we don't need to capture        */
	/* anything, but this is included for completeness.                       */
	pcre2_match_data *match_data;
	match_data = pcre2_match_data_create_from_pattern(re, NULL);

	/* Now run the match */
	int rc = pcre2_match(
			re,
			(PCRE2_SPTR) username,
			strlen(username),
			0,
			0,
			match_data,
			NULL);

	/* If the match failed, handle the errors */
	if (rc < 0){
		switch (rc) {
			case PCRE2_ERROR_NOMATCH:
				v_print("Regex did not match %s\n", username);
				break;
			default:
				v_print("Matching error %d\n", rc);
				break;
		}
		pcre2_match_data_free(match_data);
		pcre2_code_free(re);
		return false;
	}
	
	v_print("Regex matched %s\n", username);
	pcre2_match_data_free(match_data);
	pcre2_code_free(re);
	return true;
}

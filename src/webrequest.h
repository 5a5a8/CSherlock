/******************************************************************************/
/*    FILE: webrequest.h                                                      */
/*    AUTHOR: 5a5a8 <5a5a8@protonmail.com>                                    */
/*    PURPOSE: Provide web requests using libcurl to check usernames.         */
/*    LICENSE: GNU General Public Licence v3.0: see '../LICENCE' for more.    */
/******************************************************************************/

#ifndef FILE_WEBREQUEST_SEEN
#define FILE_WEBREQUEST_SEEN

#include "csv.h"
#include <stdbool.h>
#include <stdlib.h>

struct web_response {
	char *text;
	size_t length;
};

bool get_request(struct csv_columns site_data);
size_t write_data(void *text, size_t size, size_t nmemb,
				  struct web_response *response);
void init_string(struct web_response *response);

#endif

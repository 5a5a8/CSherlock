/******************************************************************************/
/*    FILE: webrequest.c                                                      */
/*    AUTHOR: 5a5a8 <5a5a8@protonmail.com>                                    */
/*    PURPOSE: Provide web requests using libcurl to check usernames.         */
/*    LICENSE: GNU General Public Licence v3.0: see '../LICENCE' for more.    */
/******************************************************************************/

#include "webrequest.h"
#include "csv.h"
#include <curl/curl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "verbose.h"

bool get_request(struct csv_columns site_data){

	bool user_found = false;

	CURL *curl = curl_easy_init();
	CURLcode result;
	curl_easy_setopt(curl, CURLOPT_URL, site_data.request_url);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L);

	/* Holds the response body in case we want to save it. */
	struct web_response response;
	init_string(&response);


	/* Some sites (e.g. Facebook) return 'Unsupported Browser' if we don't */
	/* set the User Agent to some real system.                             */
	struct curl_slist *list = NULL;
	char *head = "User-Agent: Mozilla/5.0 "
				"(Macintosh; Intel Mac OS X 10.12; rv:55.0) "
				"Gecko/20100101 Firefox/55.0";
	list = curl_slist_append(list, head);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

	/* Suppress printing response to stdout and save for later if needed */
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

	/* There are three types of errors (not including regex error handled     */
	/* before this function is called) which can indicate that a username     */
	/* does not exist on the site: status_code, message, and response_url.    */
	if (strcmp(site_data.error_type, "status_code") == 0){
		//TODO add check if text or only header is necessary
		//curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		/* We generally only need to request the headers if error is a code */
		//curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		//curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

		result = curl_easy_perform(curl);

		long http_code;
		curl_easy_getinfo(curl,	CURLINFO_RESPONSE_CODE, &http_code);

		/* Accept anything 200-299 as the username found on the site. */
		/* This is the same as the Python version of sherlock.        */
		if (http_code >= 200 && http_code < 300){
			user_found = true;
		}

	} else if (strcmp(site_data.error_type, "message") == 0){
		/* This time we need the entire text of the response */
		/* as this is where the error message will be.       */
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

		result = curl_easy_perform(curl);

		if (result != CURLE_OK){
			//TODO handle the error	
		} else {
			/* Check for the existence of the error message in the HTML */
			if (strstr(response.text, site_data.error_msg) == NULL){
				/* The error message was not found, so the user exists */
				user_found = true;	
			}
		}

	} else if (strcmp(site_data.error_type, "response_url") == 0){
		/* In this case we don't follow redirects (no-follow is the default */
		/* libcurl behaviour, so we just don't turn it on). In the case     */
		/* that the user does not exist, the site will redirect us, so we   */
		/* need to capture the status code from the original request.       */
		curl_easy_setopt(curl, CURLOPT_HEADER, 1);
		curl_easy_setopt(curl, CURLOPT_NOBODY, 1);

		result = curl_easy_perform(curl);

		long http_code;
		curl_easy_getinfo(curl,	CURLINFO_RESPONSE_CODE, &http_code);

		if (http_code >= 200 && http_code < 300){
			user_found = true;
		}
	} else {
		v_print("Unknown error type for %s: %s\n", site_data.site,
												   site_data.error_type);
	}

	/* Tidy up */
	free(response.text);
	curl_slist_free_all(list);
	curl_easy_cleanup(curl);

	return user_found;
}

size_t write_data(void *text, size_t size, size_t nmemb,
				  struct web_response *response){
	/**************************************************************************/
	/* This is the WRITEFUNCTION option for curl_easy_setopt. It stores the   */
	/* response data in a string in case we need to analyse it for some       */
	/* reason. The response struct is initialised with malloc() in the        */
	/* init_string function. Without specifying a WRITEFUNCTION, libcurl will */
	/* print the entire response text to the screen.                          */
	/**************************************************************************/

	size_t new_length = response->length + size * nmemb;
	response->text = realloc(response->text, new_length + 1);
	if (response->text == NULL){
		fprintf(stderr, "Memory reallocation failed in write_data\n");
		d_log(3, "Memory reallocation failed in curl WRITEFUNCTION\n");
		exit(EXIT_FAILURE);
	}

	memcpy(response->text + response->length, text, size * nmemb);
	response->text[new_length] = '\0';
	response->length = new_length;

	return size * nmemb;
}

void init_string(struct web_response *response){
	/* Initialises the string which will be used by the write_data() function */

	response->length = 0;
	response->text = malloc(response->length + 1);
	if (response->text == NULL){
		fprintf(stderr, "Memory allocation failed in init_string\n");
		d_log(3, "Failed to allocate memory for curl init string\n");
		exit(EXIT_FAILURE);
	}
	response->text[0] = '\0';
}

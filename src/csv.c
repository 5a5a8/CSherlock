#include "csv.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "verbose.h"

char **read_csv(char *csv_filename, int *free_length, int *lines_length){
	/**************************************************************************/
	/*   csv_filename - The name of the csv file which will be opened         */
	/*   free_length  - parameter will be modified by this function           */
	/*                   and set to the length of this functions return value */
	/*   lines_length - Parameter will be modified by this function           */
	/*                   and set to the number of lines read from the csv     */
	/*   return value - Pointer to an array of char* where each value in the  */
	/*                   array points to a line read from the csv             */
	/**************************************************************************/

	/* Initial allocation for storing the file in memory */
	int max_line_length = MAX_LINE_LEN;
	int allocated_lines = 128;
	int allocation_inc = 128;

	char **lines = malloc(allocated_lines * sizeof(char *));
	if (lines == NULL) {
		fprintf(stderr, "FATAL: Failed to allocate memory for %s\n",
				csv_filename);
		exit(EXIT_FAILURE);
	}

	/* The filename is passed in as a parameter, we open it here. */
	FILE *csv_handle = fopen(csv_filename, "r");
	if (csv_handle == NULL){
		fprintf(stderr, "FATAL: Failed to open file %s\n", csv_filename);
		exit(EXIT_FAILURE);
	}

	/* We use dynamic memory allocation as we don't know what the size */
	/* of the csv file will be in advance. Initially we allocated 128  */
	/* lines. Each time we run out of space, we allocate another 128   */
	/* lines until EOF is reached. For each line, we allow 1024 bytes. */
	int i;
	for (i=0; 1; ++i){

		/* Reallocate more memory for lines in the file if needed */
		if (i >= allocated_lines){
			allocated_lines += allocation_inc;
			lines = realloc(lines, allocated_lines * sizeof(char *));
			if (lines == NULL){
				fprintf(stderr, "FATAL: Failed to allocate memory for %s\n",
				csv_filename);
				exit(EXIT_FAILURE);
			}
		}

		/* Allocate memory for each individual line */
		lines[i] = malloc(max_line_length * sizeof(char));
		if (lines[i] == NULL){
			fprintf(stderr, "FATAL: Failed to allocate memory for %s\n",
					csv_filename);
			exit(EXIT_FAILURE);
		}

		/* Read each line and quit if we reach EOF */
		if (fgets(lines[i], max_line_length-1, csv_handle) == NULL){
			break;
		}

		/* Strip the trailing newline from each line. */
		strtok(lines[i], "\n");
	}

	/* Multiple values are returned by modifying function parameters */
	fclose(csv_handle);
	*free_length = allocated_lines;	//amount of memory the caller needs to free
	*lines_length = i;				//number of lines read
	return lines;
}

void free_csv_memory(char **lines, int free_length){
	/**************************************************************************/
	/* lines       - array of string which was the return value of read_csv() */
	/* free_length - number of memory units which must be freed               */
	/**************************************************************************/

	int i;
	for (i=0; i<free_length; ++i){
		free(lines[i]);
	}
	free(lines);
}

struct csv_columns parse_csv(char **lines, int index){

	pthread_mutex_lock(&lock);

	struct csv_columns csv_line_parsed;

	/* Avoid modifying the original line, or we get problems when    */
	/* the user passes multiple usernames as command-line arguments. */
	char linecpy_buffer[MAX_LINE_LEN];
	snprintf(linecpy_buffer, MAX_LINE_LEN, "%s", lines[index]);


	/* Get up to the first delimiter: We use ";" for delim but   */
	/* we may want to change to '|' later for wider site support */
	char csv_delim[] = ";";
	char *token = strtok(linecpy_buffer, csv_delim);


	/* Get the remaining fields */
	int i;
	char csv_array[13][MAX_FIELD_LEN];
	for (i=0; token != NULL && i<13; ++i){
		snprintf(csv_array[i], MAX_FIELD_LEN, "%s", token);
		token = strtok(NULL, csv_delim);
	}

	snprintf(csv_line_parsed.site, MAX_FIELD_LEN, "%s", csv_array[0]);
	snprintf(csv_line_parsed.error_type, MAX_FIELD_LEN, "%s", csv_array[1]);
	snprintf(csv_line_parsed.error_msg, MAX_FIELD_LEN, "%s",  csv_array[2]);
	snprintf(csv_line_parsed.regex_check, MAX_FIELD_LEN, "%s", csv_array[3]);
	snprintf(csv_line_parsed.url, MAX_FIELD_LEN, "%s", csv_array[4]);
	snprintf(csv_line_parsed.url_main, MAX_FIELD_LEN, "%s", csv_array[5]);
	snprintf(csv_line_parsed.probe_url, MAX_FIELD_LEN, "%s", csv_array[6]);
	snprintf(csv_line_parsed.user_claimed, MAX_FIELD_LEN, "%s", csv_array[7]);
	snprintf(csv_line_parsed.user_unclaimed, MAX_FIELD_LEN, "%s", csv_array[8]);
	snprintf(csv_line_parsed.error_url, MAX_FIELD_LEN, "%s", csv_array[9]);
	snprintf(csv_line_parsed.no_period, MAX_FIELD_LEN, "%s", csv_array[10]);
	snprintf(csv_line_parsed.headers, MAX_FIELD_LEN, "%s", csv_array[11]);
	snprintf(csv_line_parsed.request_head_only, MAX_FIELD_LEN, "%s",
														csv_array[12]);

	pthread_mutex_unlock(&lock);
	return csv_line_parsed;
}

int make_url(char *url, char *username, char *new_url){
	/**************************************************************************/
	/* The URL in the CSV file is like https://example.com/{}, where the {}   */
	/* should be replaced by a username. This function simply makes that      */
	/* replacement. We copy the part before, concatenate the username, and    */
	/* then concatenate the part after the closing brace.                     */
	/**************************************************************************/

	int i;

	/* Copy up to the opening brace, if we reach null byte, url is bad */
	for (i=0; url[i] != '{'; ++i){
		if (url[i] == '\0'){
			v_print("URL %s is not valid, no open brace.\n", url);
			return 1;
		}
		new_url[i] = url[i];
	}
	new_url[i] = '\0';

	/* Check for close brace */
	if (url[i+1] != '}'){
		v_print("URL %s: No close brace after opening brace.\n", url);
		return 1;
	}

	/* Append the username */
	strncat(new_url, username, MAX_FIELD_LEN-1);

	/* Add the rest of the url and the null-byte */
	i += 2;
	int j = strlen(new_url);
	for (; url[i] != '\0'; ++j){
		new_url[j] = url[i];
		++i;
	}
	new_url[j] = '\0';

	/* The new_url parameter now points to the final request url */
	return 0;
}

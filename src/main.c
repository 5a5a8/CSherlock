/******************************************************************************/
/*    CSherlock: Find usernames across social networks.                       */
/*                                                                            */
/*    This is a C rewrite of the Sherlock project:                            */
/*    https://github.com/sherlock-project/sherlock                            */
/*                                                                            */
/*    FILE: main.c                                                            */
/*    AUTHOR: 5a5a8 <5a5a8@protonmail.com>                                    */
/*    PURPOSE: Main file of CSherlock project.                                */
/*    LICENSE: GNU General Public Licence v3.0: see '../LICENCE' for more.    */
/******************************************************************************/

#include "argparser.h"
#include "csv.h"
#include "help.h"
#include <pthread.h>
#include "regexcheck.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "verbose.h"
#include "webrequest.h"

/* For coloured terminal output */
#define GREEN(string) "\x1b[32m" string "\x1b[0m"
#define RED(string) "\x1b[31m" string "\x1b[0m"

/* Arguments passed to the multithreaded function - as only one argument is   */
/* accepted for POSIX threads we must pass in a struct of all the args.       */
struct thread_args {
	bool print_all;
	bool write_csv;
	char *username;
	char **sites_list;
	int i_low;
	int i_high;
	int threadnum;
};

void log_result(bool print_all, bool write_csv, char *site,
				char *username, bool result, char *url);
void *csherlock(void *args);

int main(int argc, char *argv[]){
	/* Parse command line arguments */
	struct cmd_args args = argparser(argc, argv);

	/* If -h or -v given, just print the desired output and exit */
	if (args.help){
		print_help(argv[0]);
		return 0;
	} else if (args.version){
		printf("CSherlock v%s\n", VERSION);
		return 0;
	}

	if (args.verbose){
		set_verbose(true);
	}
	if (args.debug_log){
		set_debug(true);
	}

	/* We now read and parse the csv file containing the list of sites. */
	/* the csv functions are defined in csv.c and csv.h.                */
	/* Memory allocation is dynamic so we need to keep track of it.     */
	int i;
	int free_length;	//how much memory we need to free
	int num_lines;		//number of lines read from the csv

	/* read_csv() pulls each line into the lines array with the \n removed. */
	char **lines = read_csv("sites.csv", &free_length, &num_lines);

	/* Initialise mutex lock on csv parser, csv writer, debug log. */
	if (pthread_mutex_init(&infile_lock, NULL)){
		v_print("Failed to initialise mutex on infile\n");
		d_log(3, "Failed to initialise mutex on infile\n");
	}
	if (args.write_csv){
		if (pthread_mutex_init(&outfile_lock, NULL)){
			v_print("Failed to initialise mutex on outfile\n");
			d_log(3, "Failed to initialise mutex on outfile\n");
		}
	}
	if (args.debug_log){
		if (pthread_mutex_init(&debug_file_lock, NULL)){
			v_print("Failed to initialise mutex on debug log file\n");
			d_log(3, "Failed to initialise mutex on debug lof file\n");
		}
	}

	/* Start checking each username in the arguments */
	for (i = optind; i < argc; ++i){
		printf("Checking username %s on...\n\n", argv[i]);

		int divs = num_lines / args.num_threads;
		int remaining_sites = num_lines % args.num_threads;

		/* Set up multithreading */
		pthread_t threads[args.num_threads];
		int prev_i_high = 1;
		int j;
		for (j=0; j<args.num_threads; ++j){
			/* Multithreaded function takes one pointer argument, so we must  */
			/* pass it a struct of all the arguments. Use malloc to ensure    */
			/* that other threads will not touch the same data. The pointer   */
			/* is freed in the called function.                               */
			struct thread_args *t_data = malloc(sizeof(struct thread_args));

			t_data->print_all = args.print_all;
			t_data->write_csv = args.write_csv;
			t_data->username = argv[i];
			t_data->sites_list = lines;
			t_data->threadnum = j;
			t_data->i_low = prev_i_high;
			t_data->i_high = (t_data->i_low) + divs;

			/* Assign the left over csv lines to each thread */
			if (t_data->i_high > num_lines){
				t_data->i_high = num_lines;
			}
			if (remaining_sites){
				++(t_data->i_high);
				--remaining_sites;
			}
			prev_i_high = t_data->i_high;

			int rc;
			rc = pthread_create(&threads[j], NULL, csherlock, t_data);
			if (rc){
				v_print("Failed to start thread %d\n", j);
				d_log(2, "Failed to start thread %d\n", j);
			}
		}

		int k;
		for (k=0; k<args.num_threads; ++k){
			pthread_join(threads[k], NULL);
		}
	}

	/* Free the memory allocated by read_csv() */
	v_print("Freeing memory allocated for csv...\n");
	free_csv_memory(lines, free_length);

	return 0;
}

void *csherlock(void *args){
	/* Get our struct back as individual variables */
	struct thread_args t_data = *(struct thread_args*) args;
	bool print_all = t_data.print_all;
	bool write_csv = t_data.write_csv;
	char *username = t_data.username;
	char **sites_list = t_data.sites_list;
	int threadnum = t_data.threadnum;
	int i_low = t_data.i_low;
	int i_high = t_data.i_high;
	free(args);

	int i;
	/* We now loop through every website in the csv file, first checking the  */
	/* regex against the username if one exists. If the regex does not match  */
	/* the username, then it's an invalid username for the site, so we know   */
	/* that it does not exist on the site, avoiding the need for an http      */
	/* request. If there is no regex for the site, 'NONE' should be the value */
	/* in the csv file. In this case, we skip the regex check.                */

	for (i=i_low; i<i_high; ++i){

		/* Parse the csv line so that each field becomes an element of a */
		/* struct.                                                       */
		struct csv_columns site_data = parse_csv(sites_list, i);

		/* Check the regex against username. */
		bool regex_matched;
		if (strcmp(site_data.regex_check, "NONE") != 0){
			regex_matched = RE_check_regex(site_data.regex_check, username);
			if (!regex_matched){
				log_result(print_all, write_csv, site_data.site,
							username, false, site_data.url);
				continue;
			}
		}

		/* After the call to make_url, site_data.request_url will hold */
		/* the new URL. This is done by replacing the '{}' wildcard in */
		/* the URL with the username.                                  */
		site_data.request_url = malloc(MAX_FIELD_LEN);
		if (site_data.request_url == NULL){
			fprintf(stderr, "Out of memory\n");
			d_log(3, "Could not malloc space for request URL\n");
		}

		int fail;
		if (strcmp(site_data.probe_url, "NONE") == 0){
			fail = make_url(site_data.url, username, site_data.request_url);
		} else {
			fail = make_url(site_data.probe_url, username,
									site_data.request_url);
		}
		if (fail){
			v_print("Invalid url on line %d, thread: %d\n", i, threadnum);
			d_log(2, "Invalid URL on line %d of input CSV file, skipping...\n");
			continue;
		}

		/* If the regex check passed (or if there was no regex supplied) */
		/* then we need to make an http request.                         */
		log_result(print_all,
					write_csv,
					site_data.site,
					username,
					get_request(site_data), //make the request
					site_data.url);

		free(site_data.request_url);
	}

	return NULL;
}

void log_result(bool print_all, bool write_csv, char *site,
				char *username, bool result, char *url){

	char new_url[MAX_FIELD_LEN];
	make_url(url, username, new_url);

	if (result){
		printf("[+] " GREEN("%s") ": %s\n", site, new_url);
		if (write_csv){
			char outfile_name[260];
			strcpy(outfile_name, username);
			strcat(outfile_name, ".csv");
			write_csv_result(outfile_name, new_url);
		}
	} else if (print_all){
		printf("[-] " RED("%s") ": Not found: %s\n", site, username);
	}
}

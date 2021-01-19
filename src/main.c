/******************************************************************************/
/* CSherlock: Find usernames across social networks.                          */
/*                                                                            */
/* This is a C rewrite of the Sherlock project:                               */
/* https://github.com/sherlock-project/sherlock                               */
/*                                                                            */
/* This rewrite was created by Github user 5a5a8.                             */
/* 5a5a8@protonmail.com                                                       */
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

#define MAX_THREADS 32

/* Arguments passed to the multithreaded function - as only one argument is   */
/* accepted for POSIX threads we must pass in a struct of all the args.       */
struct thread_args {
	bool print_all;
	char *username;
	char **sites_list;
	int i_low;
	int i_high;
	int threadnum;
};

void log_result(bool print_all, char *site,
				char *username, bool result, char *url);
void *csherlock(void *args);

int main(int argc, char *argv[]){

	/* Parse command line arguments */
	struct cmd_args args = argparser(argc, argv);

	/* If -h or -v given, just print the desired output and exit */
	if (args.help) {
		print_help(argv[0]);
		return 0;
	} else if (args.version) {
		printf("CSherlock v%s\n", VERSION);
		return 0;
	}

	if (args.verbose){
		set_verbose(true);
	}

	/* We now read and parse the csv file containing the list of sites. */
	/* the csv functions are defined in csv.c and csv.h.                */ 
	/* Memory allocation is dynamic so we need to keep track of it.     */
	int i;
	int free_length;	//how much memory we need to free
	int num_lines;		//number of lines read from the csv

	/* read_csv() pulls each line into the lines array with the \n removed. */
	char **lines = read_csv("sites.csv", &free_length, &num_lines); 

	/* Start checking each username in the arguments */
	for (i = optind; i < argc; ++i){
		int divs = num_lines / MAX_THREADS;

		/* Set up multithreading */
		int j;
		pthread_t threads[MAX_THREADS];
		for (j=0; j<MAX_THREADS; ++j){
			/* Multithreaded function takes one pointer argument, so we must  */
			/* pass it a struct of all the arguments. Use malloc to ensure    */
			/* that other threads will not touch the same data. The pointer   */
			/* is freed in the called function.                               */ 
			struct thread_args *t_data = malloc(sizeof(struct thread_args));

			t_data->print_all = args.print_all;
			t_data->username = argv[i];
			t_data->sites_list = lines;
			t_data->threadnum = j;
			t_data->i_low = j * divs + 1; // +1 to skip the csv header line
			t_data->i_high = (j * divs) + divs;

			int rc;
			rc = pthread_create(&threads[j], NULL, csherlock, t_data);
			if (rc){
				v_print("Failed to start thread %d\n", j);
			}
		}

		/* Deal with the remaining sites (those not assigned to thread). */
		/* In future these should be shared across existing threads.     */
		struct thread_args *t_data = malloc(sizeof(struct thread_args));
		t_data->print_all = args.print_all;
		t_data->username = argv[i];
		t_data->sites_list = lines;
		t_data->threadnum = -1;
		t_data->i_low = num_lines - num_lines % MAX_THREADS;
		t_data->i_high = num_lines;

		/* Make the web request */
		csherlock(t_data);

		for (j=0; j<MAX_THREADS; ++j){
			pthread_join(threads[j], NULL);
		}
		pthread_exit(NULL);
	}

	/* Free the memory allocated by read_csv() */
	free_csv_memory(lines, free_length);

	return 0;
}

void *csherlock(void *args){

	/* Get our struct back as individual variables */
	struct thread_args t_data = *(struct thread_args*) args;
	bool print_all = t_data.print_all;
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
				log_result(print_all, site_data.site,
							username, false, site_data.url);
				continue;
			}
		}

		/* After the call to make_url, site_data.request_url will hold */
		/* the new URL. This is done by replacing the '{}' wildcard in */
		/* the URL with the username.                                  */ 
		site_data.request_url = malloc(256);
		if (site_data.request_url == NULL){
			fprintf(stderr, "Out of memory\n");
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
			continue;
		}

		/* If the regex check passed (or if there was no regex supplied) */
		/* then we need to make an http request.                         */ 
		log_result(print_all,
					site_data.site,
					username,
					get_request(site_data), //make the request
					site_data.url);

		free(site_data.request_url);
	}

	return NULL;
}

void log_result(bool print_all, char *site,
				char *username, bool result, char *url){

	char new_url[256];
	make_url(url, username, new_url);

	if (result){
		printf("User %s found on %s at: %s\n", username, site, new_url);
	} else if (print_all){
		printf("User %s not found on %s\n", username, site);
	}
}

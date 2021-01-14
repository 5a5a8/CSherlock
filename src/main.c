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

/* Arguments passed to the multithreading function, as only one argument is
   accepted we must pass in a struct of all the args. */
struct thread_args {
	bool print_all;
	char *username;
	char **sites_list;
	int num_sites;
	int i_low;
	int i_high;
	int threadnum;
};

int csherlock(bool print_all, char *username, char **sites_list, int num_sites);
void log_result(bool print_all, char *site,
				char *username, bool result, char *url);
void *multithread_csherlock(void *args);

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

	/* Start checking each username against each site */
	for (i = optind; i < argc; ++i){
		csherlock(args.print_all, argv[i], lines, num_lines);
	}

	/* Free the memory allocated by read_csv() */
	free_csv_memory(lines, free_length);

	return 0;
}

int csherlock(bool print_all, char *username, char **sites_list, int num_sites){
	/**************************************************************************/
	/* This function sets up multithreading and will be moved to main()       */
	/**************************************************************************/

	int divs = num_sites / MAX_THREADS;

	int i;
	pthread_t threads[MAX_THREADS];
	for (i=0; i<MAX_THREADS; ++i){
		/* Multithreaded function takes one pointer argument, so we must
		   pass it a struct of all the arguments. Use malloc to ensure
		   that other threads will not touch the same data. */

		/* This malloc is freed by the called function */
		struct thread_args *t_data = malloc(sizeof(struct thread_args));

		t_data->print_all = print_all;
		t_data->username = username;
		t_data->sites_list = sites_list;
		t_data->num_sites = num_sites;
		t_data->threadnum = i;
		t_data->i_low = i * divs + 1; // +1 to skip the csv header line
		t_data->i_high = (i * divs) + divs;

		int rc;
		rc = pthread_create(&threads[i], NULL, multithread_csherlock, t_data);
		if (rc){
			v_print("Failed to start thread %d\n", i);
		}
	}

	/* Deal with the remaining sites (those not assigned to thread) in main. */
	/* In future we should just distribute the remainder across the threads. */
	struct thread_args *t_data = malloc(sizeof(struct thread_args));
	t_data->print_all = print_all;
	t_data->username = username;
	t_data->sites_list = sites_list;
	t_data->num_sites = num_sites;
	t_data->threadnum = -1;
	t_data->i_low = num_sites - num_sites % MAX_THREADS;
	t_data->i_high = num_sites;

	multithread_csherlock(t_data);

	for (i=0; i<MAX_THREADS; ++i){
		pthread_join(threads[i], NULL);
	}
	pthread_exit(NULL);

	return 0;
}

void *multithread_csherlock(void *args){

	/* Get our struct back as individual variables */
	struct thread_args t_data = *(struct thread_args*) args;
	bool print_all = t_data.print_all;
	char *username = t_data.username;
	char **sites_list = t_data.sites_list;
	int num_sites = t_data.num_sites;
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
		/* struct. We then build the request URL which replaces the '{}' */
		/* wildcard in the URL with the username. If a probe URL exists, */
		/* use that to build the request URL.                            */ 
		struct csv_columns site_data = parse_csv(sites_list, i);
		site_data.request_url = malloc(256);
		if (site_data.request_url == NULL){
			fprintf(stderr, "Out of memory\n");
		}
		
		/* After the call to make_url, site_data.request_url will hold */
		/* the new URL.                                                */
		int fail;
		if (strcmp(site_data.probe_url, "NONE") == 0){
			fail = make_url(site_data.url, username, site_data.request_url);
		} else {
			fail = make_url(site_data.probe_url, username, site_data.request_url);
		}
		if (fail){
			v_print("Invalid url on line %d\n", i);
			continue;
		}

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

		/* If the regex check passed (or there was no regex supplied) */
		/* then we need to make an http request.                      */ 
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

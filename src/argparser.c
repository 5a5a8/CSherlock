#include "argparser.h"
#include "help.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct cmd_args argparser(int argc, char **argv){

	/* Set the default arguments */
	struct cmd_args args;
	args.help = false;
	args.version = false;
	args.print_all = false;
	args.verbose = false;

	/* Often users will just do --help straight away */
	/* or run the program with no args.              */
	if (argc > 1 && strcmp(argv[1], "--help") == 0){
		print_help(argv[0]);
		exit(EXIT_SUCCESS);
	} else if (argc == 1){
		printf("Usage: %s [-%s] [usernames]\n", argv[0], ARGS);
		printf("Use --help for more info\n");
		exit(EXIT_SUCCESS);
	}

	/* We set each variable as per user's arguments and     */
	/* avoid calling any functions until we parse all args. */
	int opt;
	while ((opt = getopt(argc, argv, "hvVp")) != -1){
		switch (opt){
			case 'h':
				args.help = true;
				break;
			case 'v':
				args.verbose = true;
				break;
			case 'V':
				args.version = true;
				break;
			case 'p':
				args.print_all = true;
				break;
			default:
				fprintf(stderr, "Usage: %s [-%s] [usernames]\n", argv[0], ARGS);
				exit(EXIT_FAILURE);
		}
	}

	/* optind contains index of first non-option argument.     */
	/* If it is >= argc, the user did not enter any usernames. */
	/* This is ok if -V or -h was given, but everything else   */
	/* requires a username to be entered.                      */
	if (optind >= argc && !(args.help || args.version)){
		fprintf(stderr, "Usage: %s [-%s] [usernames]\n", argv[0], ARGS);	
		fprintf(stderr, "Please ensure at least one username was entered.\n");
		exit(EXIT_FAILURE);
	}
	return args;
}

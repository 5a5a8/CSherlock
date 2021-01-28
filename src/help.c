/******************************************************************************/
/*    FILE: help.c                                                            */
/*    AUTHOR: 5a5a8 <5a5a8@protonmail.com>                                    */
/*    PURPOSE: Print help message to the screen.                              */
/*    LICENSE: GNU General Public Licence v3.0: see '../LICENCE' for more.    */
/******************************************************************************/

#include "help.h"
#include <stdio.h>

void print_help(char *program_name){
	printf("Usage: %s [-hvVp] [usernames]\n\n", program_name);

	printf("CSherlock: C Rewrite of Sherlock: "
			"Find Usernames Across Social Networks\n");
	printf("Version %s\n\n", VERSION);

	printf("Positional Arguments:\n");
	printf("  usernames\t" 
			"One or more usernames to check with social networks.\n\n");

	printf("Optional Arguments:\n");
	printf("  -h\t\tShow this help message and exit.\n");
	printf("  -V\t\tDisplay version information and exit.\n");
	printf("  -v\t\tDisplay extra debug info and metrics.\n");
	printf("  -p\t\tPrint all sites, including where user not found.\n");
	printf("  -c\t\tCreate a 'username.csv' file of the result\n");
}

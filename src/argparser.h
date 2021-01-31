/******************************************************************************/
/*    FILE: argparser.h                                                       */
/*    AUTHOR: 5a5a8 <5a5a8@protonmail.com>                                    */
/*    PURPOSE: Parse command line arguments                                   */ 
/*    LICENSE: GNU General Public Licence v3.0: see '../LICENCE' for more.    */
/******************************************************************************/

#ifndef FILE_ARGPARSER_SEEN
#define FILE_ARGPARSER_SEEN

#include <stdbool.h>

#define ARGS "hvVpctd"
#define MAX_THREADS 256

struct cmd_args {
	bool help;
	bool version;
	bool print_all;
	bool verbose;
	bool write_csv;
	bool threads;
	long num_threads;
	bool debug_log;
};

struct cmd_args argparser(int argc, char **argv);

#endif

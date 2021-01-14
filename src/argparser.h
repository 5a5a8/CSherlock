#ifndef FILE_ARGPARSER_SEEN
#define FILE_ARGPARSER_SEEN

#include <stdbool.h>

#define ARGS "hvVp"

struct cmd_args {
	bool help;
	bool version;
	bool print_all;
	bool verbose;
};

struct cmd_args argparser(int argc, char **argv);

#endif

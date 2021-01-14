#ifndef FILE_CSV_SEEN
#define FILE_CSV_SEEN

struct csv_columns {
	char site[256];
	char error_type[256];
	char error_msg[256];
	char regex_check[256];
	char url[256];
	char url_main[256];
	char probe_url[256];
	char user_claimed[256];
	char user_unclaimed[256];
	char error_url[256];
	char no_period[256];
	char headers[256];
	char request_head_only[256];
	
	char *request_url;
};

char **read_csv(char *csv_filename, int *free_length, int *lines_length);
void free_csv_memory(char **lines, int free_length);
struct csv_columns parse_csv(char **lines, int index);
int make_url(char *url, char *username, char *new_url);

#endif

#ifndef FILE_CSV_SEEN
#define FILE_CSV_SEEN

#define MAX_LINE_LEN 1024
#define MAX_FIELD_LEN 256 

struct csv_columns {
	char site[MAX_FIELD_LEN];
	char error_type[MAX_FIELD_LEN];
	char error_msg[MAX_FIELD_LEN];
	char regex_check[MAX_FIELD_LEN];
	char url[MAX_FIELD_LEN];
	char url_main[MAX_FIELD_LEN];
	char probe_url[MAX_FIELD_LEN];
	char user_claimed[MAX_FIELD_LEN];
	char user_unclaimed[MAX_FIELD_LEN];
	char error_url[MAX_FIELD_LEN];
	char no_period[MAX_FIELD_LEN];
	char headers[MAX_FIELD_LEN];
	char request_head_only[MAX_FIELD_LEN];
	
	char *request_url;
};

char **read_csv(char *csv_filename, int *free_length, int *lines_length);
void free_csv_memory(char **lines, int free_length);
struct csv_columns parse_csv(char **lines, int index);
int make_url(char *url, char *username, char *new_url);

#endif

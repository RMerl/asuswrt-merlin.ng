#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <regex.h>

#define SYSLOG_TYPE     1
#define FILE_TYPE       2

const char* ident =  "colog";
int logopt = LOG_PID | LOG_CONS;
int facility = LOG_USER;
int priority = LOG_ERR | LOG_USER;
int g_log_fd;
int g_stream_type = 0;
char g_ident[10] = {0};
regex_t pattern;
int ignore_case = 1;
int extended = 1;
int errors = 0;
int g_do_regexec = -1;

int rotate_log_file(const char* log_file, int log_file_size, int log_rotate_number){
        
	if( (g_stream_type & FILE_TYPE) != FILE_TYPE ) 
		return -1;

	int i = 0;
        size_t size = 0;
        char buf[1024] = {0};

        struct stat tmp_log_stat;
        int tmp_stat = stat(log_file, &tmp_log_stat);
        size = tmp_log_stat.st_size;

        if( size > log_file_size ){
                for(i=log_rotate_number; i>=0; i--){
                        
			char the_log_file[20] = {0};
			
			if(i==0){
				sprintf(the_log_file, "%s", log_file);
			}
			else{
                        	sprintf(the_log_file, "%s-%d", log_file, i);
			}

                        FILE* pfile;
                        pfile = fopen(the_log_file, "r");
                        if(pfile){
                                if(i==log_rotate_number){
                                        unlink(the_log_file);
                                }
                                else{
                                        char new_log_file[20] = "";
                                        sprintf(new_log_file, "%s-%d", log_file, i+1);
                                        rename(the_log_file, new_log_file);
                                }

                                fclose(pfile);
                        }
                }

                //rename(log_file, "/tmp/hyd_log-1");

		if((g_log_fd = open(log_file, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0){
                	printf("Fail to open log file!");
                        return -1;
                }
        }

	return 1;
}

char *trimwhitespace(char *str) {
	char *end;

  	// Trim leading space
	while(isspace((unsigned char)*str)) str++;

  	if(*str == 0)  // All spaces?
    		return str;

  	// Trim trailing space
  	end = str + strlen(str) - 1;
  	while(end > str && isspace((unsigned char)*end)) end--;

  	// Write new null terminator
  	*(end+1) = '\n';

  	return str;
}

int compile_pattern(const char *pat) {
	if(pat==NULL) return -1;
	int flags = REG_NOSUB;
	int ret;
	#define MSGBUFSIZE 512
	char error[MSGBUFSIZE];
	if (ignore_case)
		flags |= REG_ICASE;
	if (extended)
		flags |= REG_EXTENDED;
	ret = regcomp(&pattern, pat, flags);
	return ret;
}

int main(int argc, char **argv){

	int i = 0, opt;
	int log_file_size = 10240;//524288;
        int log_rotate_number = 5;
	size_t size = 0;
	char log_file[20] = {0};

	strncpy(g_ident, ident, 10);

	while( (opt = getopt(argc, argv, "ofp:n:s:r:")) != -1 ) {

		switch(opt){
		case 'p':
			strncpy(g_ident, optarg, 10);
			break;

		case 'f':
			g_stream_type |= FILE_TYPE;
			break;

		case 'n':
			log_rotate_number = atoi(optarg);
			break;

		case 's':
			log_file_size = atoi(optarg);
			break;
		
		case 'o':
			g_stream_type |= SYSLOG_TYPE;
			break;

		case 'r':
			g_do_regexec = compile_pattern(optarg);
			break;

		case '?':
			printf("Unknown option: %c\n",(char)optopt);
			break;
		}
	}
		
	if( (g_stream_type & SYSLOG_TYPE) == SYSLOG_TYPE ) {
		openlog(g_ident, logopt, facility);
		printf("Output to system log.\n");	
	}
		
	if( (g_stream_type & FILE_TYPE) == FILE_TYPE ) {
		sprintf(log_file, "/tmp/%s_log", g_ident);

        	if((g_log_fd = open(log_file, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0){
        		printf("Fail to open log file[%s]!", log_file);
                	return -1;
        	}

		printf("Output to log file[%s], log_size=%d, rotate=%d.\n", log_file, log_file_size, log_rotate_number);
	}

	char* line = NULL;

	while( getline(&line, &size, stdin) != -1 ) {

		if(g_do_regexec==0){
			if (regexec(&pattern, line, 0, NULL, 0) != 0) {
                        	//printf("no match %s\n", line);
                        	continue;
                	}
		}

		if( (g_stream_type & SYSLOG_TYPE) == SYSLOG_TYPE ) {
			syslog(priority, "%s", line);
                }
	
		if( (g_stream_type & FILE_TYPE) == FILE_TYPE ) {
                        rotate_log_file(log_file, log_file_size, log_rotate_number);
			line = trimwhitespace(line);
			
			if(strlen(line)>0)
				write(g_log_fd, line, strlen(line));

                }
	}

	regfree(&pattern);

	if(line!=NULL){
		free(line);
		line = NULL;
	}

	if( (g_stream_type & SYSLOG_TYPE) == SYSLOG_TYPE ) {	
		closelog();
        }
	
	if( (g_stream_type & FILE_TYPE) == FILE_TYPE ) {
		close(g_log_fd);
		g_log_fd = -1;
	}

	return 0;
}

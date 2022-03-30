/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <linux/ctype.h>
#include <stdlib.h>
#include <string.h>
#include "bca_common.h"

#define CLI_CB_NUM 8

struct cli_job_cb
{
    unsigned long last_time;
    unsigned long time_period;
    void (*job_cb)(void);
};

static unsigned long        registred_cb_count = 0;
static struct cli_job_cb    cli_job_cb_arr[CLI_CB_NUM];

void init_cli_cb_arr(void)
{
    memset(cli_job_cb_arr, 0, sizeof(struct cli_job_cb)*CLI_CB_NUM);
}

void register_cli_job_cb(unsigned long time_period, void (*job_cb)(void))
{
    int i;
    for(i=0; i<CLI_CB_NUM; i++) 
    {
        if (!cli_job_cb_arr[i].job_cb)
        {
            cli_job_cb_arr[i].job_cb = job_cb;
            cli_job_cb_arr[i].time_period = time_period;
            cli_job_cb_arr[i].last_time = get_timer(0);
            registred_cb_count++;
            break;
        }
    }
}

void unregister_cli_job_cb(void (*job_cb)(void))
{
    int i,j;
    for(i=0; i<registred_cb_count; i++) 
    {
        if (cli_job_cb_arr[i].job_cb == job_cb)
        {
            for(j=i; j<registred_cb_count; j++)
            {
                if(j+1 == registred_cb_count)
                {
                    memset(&cli_job_cb_arr[j], 0, sizeof(struct cli_job_cb));
                    break;
                }
                memcpy(&cli_job_cb_arr[j], &cli_job_cb_arr[j+1], sizeof(struct cli_job_cb));
            }
            registred_cb_count--;
            break;
        }
    }
}

void run_cli_jobs(void)
{
    int i;
    for(i=0; i<registred_cb_count; i++) 
    {
        if(cli_job_cb_arr[i].job_cb)
        {
            if(!cli_job_cb_arr[i].time_period)
            {
                cli_job_cb_arr[i].job_cb();
            }
            else
            {
                if(get_timer(cli_job_cb_arr[i].last_time) >= cli_job_cb_arr[i].time_period)
                {
                    cli_job_cb_arr[i].job_cb();
                    cli_job_cb_arr[i].last_time = get_timer(0);
                }
            }
        }
    }
}

int suffix2shift(char suffix)
{
	if (suffix == 'K')
	{
		return(10);
	} else if (suffix == 'M')
	{
		return(20);
	} else if (suffix == 'G')
	{
		return(30);
	}
	return(0);
}

/**
 * parse_env_nums - get named environment value of format FIELD=n1,n2,n3....
 * @buffer: pointer environment variable to parse
 * @maxargs: max number of arguments to parse
 * @args: pointer to array of unsigned longs for numeric arguments
 * @suffixes: pointer to array of chars for suffix characters
 *
 * returns:
 *     number of arguments total if successful
 *     0 if not found
 */
int parse_env_nums(const char *buffer, const int maxargs, unsigned long *args, char *suffixes)
{
	int ret = 0;
	char *b = NULL;
	char *p;
	int i;
	int l;
	char *tok;
	if (NULL != buffer)
	{
		l = strlen(buffer);
		b = malloc(l+1); 
		strncpy(b, buffer, l+1);
		p = b;
		for (i = 0 ; i < maxargs ; i++)
		{
			tok = strtok(p,",");
			p = NULL;
			if (NULL != tok)
			{
				char *cp = NULL;
				args[i] = simple_strtoul(tok, &cp, 0);
				suffixes[i] = '\0';
				if (NULL != cp)
				{
					if (isalpha(*cp) && (NULL != suffixes))
					{
						suffixes[i] = toupper(*cp);
					} 
				}
				ret++;
			}
			else
			{
				break;
			}
		
		}
	}
	if (b) free(b);
	return(ret); 
}


/**
 * parse_env_string_plus_nums - get named environment value of format FIELD=name:n1,n2,n3....
 * @buffer: pointer environment variable to parse
 * @name: pointer to buffer to which name will be copied [ caller is required to free this pointer ]
 * @maxargs: max number of arguments to parse
 * @args: pointer to array of unsigned longs for numeric arguments
 * @suffixes: pointer to array of chars for suffix characters
 *
 * returns:
 *     number of arguments total  (name + args) if successful
 *     0 if not found
 */
int parse_env_string_plus_nums(const char *buffer, char **name, const int maxargs, unsigned long *args, char *suffixes)
{
	int ret = 0;
	char *b;
	int i;
	int l;
	char *tok;
	if (NULL != buffer)
	{
		l = strlen(buffer);
		b = malloc(l+1); 
		strncpy(b, buffer, l+1);
		tok = strtok(b,":");
		*name = tok; 
		ret++;
		for (i = 0 ; i < maxargs ; i++)
		{
			tok = strtok(NULL,",");
			if (NULL != tok)
			{
				char *cp = NULL;
				args[i] = simple_strtoul(tok, &cp, 0);
				suffixes[i] = '\0';
				if (NULL != cp)
				{
					if (isalpha(*cp) && (NULL != suffixes))
					{
						suffixes[i] = toupper(*cp);
					} 
				}
				ret++;
			}
			else
			{
				break;
			}
		
		}
	}
	return(ret); 
}


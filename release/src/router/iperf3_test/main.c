#include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>



struct iperf3_param {
	int transmit_time;
	int interval_time;
	char ip[32];
	char parse_file[64];
};

void* parse_iperf3_client(void *data) 
{


	struct iperf3_param *iperf3 = (struct iperf3_param*) data;

	// printf("iperf3->ip = %s\n", iperf3->ip);
	// printf("iperf3->parse_file = %s\n", iperf3->parse_file);
	// printf("iperf3->transmit_time = %d\n", iperf3->transmit_time);
	// printf("iperf3->interval_time = %d\n", iperf3->interval_time);

	iperf3_client(iperf3->ip, iperf3->parse_file, iperf3->transmit_time, iperf3->interval_time);

	pthread_exit(NULL);

}


void iperf3_client_start(const char* ip, const char* parse_file, 
						 const int transmit_time, const int interval_time)
{


	pthread_t sockThread;
	struct iperf3_param iperf3;

	iperf3.transmit_time = transmit_time;
	iperf3.interval_time = interval_time;
	snprintf(iperf3.ip, 32, "%s", ip);
	snprintf(iperf3.parse_file, 64, "%s", parse_file);

	// printf("iperf3.interval_time = %d\n", iperf3.interval_time);
	// printf("iperf3.transmit_time = %d\n", iperf3.transmit_time);
	// printf("iperf3.ip = %s\n", iperf3.ip);
	// printf("iperf3.parse_file = %s\n", iperf3.parse_file);


	/* start thread to receive packet */
	if (pthread_create(&sockThread, NULL, parse_iperf3_client, &iperf3) < 0) {
		printf("could not create thread for iperf3 sockThread");
	}

	printf("sockThread = %u\n",  (unsigned int)sockThread);

	pthread_join(sockThread, NULL);

}

int iperf3_client(const char* ip, const char* parse_file, 
				  const int transmit_time, const int interval_time) 
{

	int transmit_count = transmit_time/interval_time;

	// del tmp file (last file)
	char parse_file_tmp[64];
	snprintf(parse_file_tmp, 64, "%s_tmp", parse_file);
	remove(parse_file_tmp);
	remove(parse_file);

	char command_t[256];	// -w 100M
	snprintf(command_t, 256, "iperf3 -c %s -t %d -i %d --logfile %s & ", ip, transmit_time, interval_time, parse_file_tmp);

	int sys_code = system(command_t);

	printf("iperf3 command_t = %s, sys_code = %d\n", command_t, sys_code);
	sleep(2);


	int count = 0;
	int file_size_tmp = 0;
	int parse_count = 0;


	while(1) {

		count++;

		if(!pids("iperf3")) {
			printf("exit iperf3 parse\n");
			break;
		}



		struct stat st;
		stat(parse_file_tmp, &st);
		int file_size = st.st_size;

		// printf("parse file_size = %d, tmp file size = %d\n", file_size, file_size_tmp);

		// waiting iperf3 output
		if(file_size > file_size_tmp) {
			parse_count++;
			file_size_tmp = file_size;
			count = 0;
		} else {
			if(count == 1) {
				printf("\nwaiting iperf3 output data [%d], about %d seconds\n", parse_count+1, interval_time);	
			}
			sleep(1);
			continue;
		}


		FILE *fp;
		char file_info[1023];

		if ((fp = fopen(parse_file_tmp, "r")) == NULL) {
			printf("%s open_file_error", parse_file_tmp);
			break;
		}

	    char transfer_interval[32];
	    int transfer_time = 0;
	    char transfer_size[64];
	    char average_rate[64];

	  	int i = 0;

		while(fgets(file_info, 1023, fp) != NULL) {

			// printf("file_info = %s", file_info);

			i++;
			if(i < (3+parse_count)) {
			  continue;
			}

			if(strstr(file_info, "- - - - - - - - -")) {
				printf("iperf3 parse end\n");
				break;
			}

			char data_tmp1[32];
			char data_tmp2[32];
			char sec_unit[32];
			char transmit_size[32];
			char transmit_unit[32];
			char transfer_rate[32];
			char transfer_rate_unit[32];

			sscanf(file_info, "%s %s %s %s %s %s%s %s", data_tmp1, data_tmp2, transfer_interval, sec_unit, transmit_size, transmit_unit, transfer_rate, transfer_rate_unit);

			float f_start_time, f_end_time;
			sscanf(transfer_interval, "%f-%f", &f_start_time, &f_end_time);


			int start_time =  (int) f_start_time; 
			int end_time =  (int) f_end_time; 
			transfer_time = end_time - start_time;
			snprintf(transfer_size, 64, "%s %s", transmit_size, transmit_unit);
			snprintf(average_rate, 64, "%s %s", transfer_rate, transfer_rate_unit);

			printf("transfer_interval = %s\n", transfer_interval);
			printf("transfer_time = %d\n", transfer_time);
			printf("transfer_size = %s\n", transfer_size);
			printf("average_rate = %s\n", average_rate);

			FILE *pFile;

			pFile = fopen( parse_file, "a+" );

			if( NULL == pFile ){
				printf("%s -> open failure\n", parse_file);
				break;

			}else{
				fprintf(pFile, "%s,%d,%s,%s\n", transfer_interval, transfer_time, transfer_size, average_rate);
			}

			fclose(pFile);
		}

		fclose(fp);


		if(parse_count >= transmit_count) {
			printf("exit iperf3 parse, parse_count(%d) >= transmit_count(%d)\n", parse_count, transmit_count);
			break;
		}
	}

	return 0;
}



void* parse_iperf3_server(void *data) 
{


	struct iperf3_param *iperf3 = (struct iperf3_param*) data;

	// printf("iperf3->ip = %s\n", iperf3->ip);
	printf("iperf3->parse_file = %s\n", iperf3->parse_file);
	printf("iperf3->transmit_time = %d\n", iperf3->transmit_time);
	printf("iperf3->interval_time = %d\n", iperf3->interval_time);

	iperf3_server(iperf3->parse_file, iperf3->transmit_time, iperf3->interval_time);

	pthread_exit(NULL);

}


void iperf3_server_start(const char* parse_file, const int transmit_time,
				         const int interval_time)
{


	pthread_t sockThread;
	struct iperf3_param iperf3;

	iperf3.transmit_time = transmit_time;
	iperf3.interval_time = interval_time;
	// snprintf(iperf3.ip, 32, "%s", ip);
	snprintf(iperf3.parse_file, 64, "%s", parse_file);

	printf("iperf3.interval_time = %d\n", iperf3.interval_time);
	printf("iperf3.transmit_time = %d\n", iperf3.transmit_time);
	// printf("iperf3.ip = %s\n", iperf3.ip);
	printf("iperf3.parse_file = %s\n", iperf3.parse_file);


	/* start thread to receive packet */
	if (pthread_create(&sockThread, NULL, parse_iperf3_server, &iperf3) < 0) {
		printf("could not create thread for iperf3 sockThread");
	}

	printf("sockThread = %u\n",  (unsigned int)sockThread);

	pthread_join(sockThread, NULL);

}

int iperf3_server(const char* parse_file, const int transmit_time,
				  const int interval_time) 
{

	// del tmp file (last file)
	char parse_file_tmp[64];
	snprintf(parse_file_tmp, 64, "%s_tmp", parse_file);
	remove(parse_file_tmp);
	remove(parse_file);

	char command_t[256];
	snprintf(command_t, 256, "iperf3 -s -i %d --logfile %s & ", interval_time, parse_file_tmp);

	int sys_code = system(command_t);

	printf("iperf3 command_t = %s, sys_code = %d\n", command_t, sys_code);
	sleep(2);

	time_t iperf3_start_time = time(NULL);
	time_t iperf3_end_time = iperf3_start_time + transmit_time;

	int count = 0;
	int file_size_tmp = 0;
	int parse_count = 0;

	while(1) {

		count++;

		if(!pids("iperf3")) {
			printf("exit iperf3 parse\n");
			break;
		}

		time_t iperf3_running_time = time(NULL);

		printf("iperf3_start_time : %ld, iperf3_running_time : %ld, iperf3_end_time : %ld\n", iperf3_start_time, iperf3_running_time, iperf3_end_time);

		if(iperf3_running_time >= iperf3_end_time) {
			printf("waiting time end, exit iperf3 parse\n");
			break;
		}

		struct stat st;
		stat(parse_file_tmp, &st);
		int file_size = st.st_size;

		// printf("parse file_size = %d, tmp file size = %d\n", file_size, file_size_tmp);

		// waiting iperf3 output
		if(file_size > file_size_tmp) {
			parse_count++;
			file_size_tmp = file_size;
			count = 0;
			iperf3_end_time = iperf3_end_time + transmit_time;
		} else {
			if(count == 1) {
				printf("\nwaiting iperf3 recive data [%d] \n", parse_count+1);	
			}
			sleep(2);
			continue;
		}


		FILE *fp;
		char file_info[1023];

		if ((fp = fopen(parse_file_tmp, "r")) == NULL) {
			printf("%s open_file_error", parse_file_tmp);
			break;
		}

	    char transfer_interval[32];
	    int transfer_time = 0;
	    char transfer_size[64];
	    char average_rate[64];

	  	int i = 0, read_exit = -1;


		while(fgets(file_info, 1023, fp) != NULL) {

			// printf("file_info = %s", file_info);

			i++;
			if(i < (6+parse_count)) {
			  continue;
			}

			if(strstr(file_info, "- - - - - - - - -")) {
				printf("iperf3 parse end\n");
				read_exit = 0;
				break;
			}

			char data_tmp1[32];
			char data_tmp2[32];
			char sec_unit[32];
			char transmit_size[32];
			char transmit_unit[32];
			char transfer_rate[32];
			char transfer_rate_unit[32];

			sscanf(file_info, "%s %s %s %s %s %s%s %s", data_tmp1, data_tmp2, transfer_interval, sec_unit, transmit_size, transmit_unit, transfer_rate, transfer_rate_unit);

			float f_start_time, f_end_time;
			sscanf(transfer_interval, "%f-%f", &f_start_time, &f_end_time);


			int start_time =  (int) f_start_time; 
			int end_time =  (int) f_end_time; 
			transfer_time = end_time - start_time;
			snprintf(transfer_size, 64, "%s %s", transmit_size, transmit_unit);
			snprintf(average_rate, 64, "%s %s", transfer_rate, transfer_rate_unit);

			printf("transfer_interval = %s\n", transfer_interval);
			printf("transfer_time = %d\n", transfer_time);
			printf("transfer_size = %s\n", transfer_size);
			printf("average_rate = %s\n", average_rate);

			FILE *pFile;

			pFile = fopen( parse_file, "a+" );

			if( NULL == pFile ){
				printf("%s -> open failure\n", parse_file);
				break;

			}else{
				fprintf(pFile, "%s,%d,%s,%s\n", transfer_interval, transfer_time, transfer_size, average_rate);
			}

			fclose(pFile);
		}

		fclose(fp);

		if(read_exit == 0) {
			printf("exit iperf3 parse\n");
			break;
		}


	}

	system("killall iperf3");

	return 0;
}


int main(int argc, char **argv) {

	// thread
	iperf3_client_start("192.168.2.1", "/tmp/iperf3_parse", 120, 10);
	// iperf3_server_start("/tmp/iperf3_server_parse", 60, 5);

	// function 
	// iperf3_client("192.168.2.1", "/tmp/iperf3_parse", 120, 10);
	// iperf3_server("/tmp/iperf3_server_parse", 60, 5);

	return 0;
}


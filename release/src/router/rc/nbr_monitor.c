int nbr_monitor_main(int argc, char *argv[])
{
	int rrm_nbr_count=0;
	int rrm_nbr_init=0;

	while(1){
		if(rrm_nbr_init == 0){
			rrm_nbr_init = 1;
			sleep(240);
		}

		if(rrm_nbr_count < 5) {
			rrm_nbr_count++;
			sleep(10);
		} else {
			rrm_nbr_count = 0;
			wl_set_nbr_info();
		}
	}

}
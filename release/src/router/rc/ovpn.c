/*
 * Copyright 2018, ASUSTeK Inc.
 * All Rights Reserved.
 *
 */

#include "rc.h"

int ovpn_up_main(int argc, char **argv)
{
//	ovpn_up_handler();

	update_resolvconf();

	return 0;
}

int ovpn_down_main(int argc, char **argv)
{
//	ovpn_down_handler();

	update_resolvconf();

	return 0;
}


int ovpn_route_up_main(int argc, char **argv)
{
//	ovpn_route_up_handler();

	return 0;
}


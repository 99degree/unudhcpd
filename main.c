// Copyright 2021 Clayton Craft <clayton@craftyguy.net>
// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "server.h"

void usage() {
	printf("Usage:\n");
	printf("\tunudhcpd -i <interface> [-s <server IP>] [-p <server port] [-c <client IP>]\n");
	printf("Where:\n");
	printf("\t-i  network interface to bind to\n");
	printf("\t-s  server IP {default: 172.168.1.1}\n");
	printf("\t-p  server port {default: 67}\n");
	printf("\t-c  client IP to issue for DHCP requests {default: 172.168.1.2}\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	dhcp_config config;
	config.server_ip = "172.168.1.1";
	config.client_ip = "172.168.1.2";
	config.server_port = 67;
	config.iface = NULL;

	int c;
	while ((c = getopt(argc, argv, "i:c:s:p:")) != -1)
		switch (c)
		{
			case 'i':
				config.iface = optarg;
				break;
			case 'c':
				config.client_ip = optarg;
				break;
			case 's':
				config.server_ip = optarg;
				break;
			case 'p':
				config.server_port = atoi(optarg);
				if (!config.server_port)
					usage();
				break;
			default:
				usage();
		}

	if (config.iface == NULL)
		usage();
	
	if (dhcp_server_start(&config) != 0)
		return 1;
	return 0;

}

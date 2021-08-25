// Copyright 2021 Clayton Craft <clayton@craftyguy.net>
// SPDX-License-Identifier: GPL-3.0-or-later
#include <arpa/inet.h>
#include <errno.h>
#include <net/if_arp.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"

char *mac_to_str(uint8_t *mac) {
	static char str[20];
	snprintf(str, 20, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return str;
}

int add_arp_entry(dhcp_config *config, uint8_t *mac, int mac_len, uint32_t ip) {
	struct arpreq areq;
	memset(&areq, 0, sizeof(areq));

	strlcpy(areq.arp_dev, config->iface, sizeof(areq.arp_dev));

	struct sockaddr_in *sock_addr = (struct sockaddr_in *) &areq.arp_pa;
	sock_addr->sin_family = AF_INET;
	sock_addr->sin_addr.s_addr = ip;

	memcpy(areq.arp_ha.sa_data, mac, mac_len);
	// completed ARP entry
	areq.arp_flags = ATF_COM;

	if (ioctl(config->server_sock, SIOCSARP, (char *)&areq) < 0)  {
		perror("Unable to add entry to ARP table");
		return 1;
	}    
	return 0;
}

int dhcp_create_response(dhcp_config *config, dhcp_header *request, dhcp_header *response, unsigned char type) {
	response->op = BOOTREPLY;
	response->htype = request->htype;
	response->hlen = request->hlen;
	memcpy(&response->xid, &request->xid, sizeof(request->xid));
	memcpy(&response->flags, &request->flags, sizeof(request->flags));
	memcpy(&response->giaddr, &request->giaddr, sizeof(request->giaddr));
	memcpy(response->chaddr, request->chaddr, request->hlen);
	memset(&response->flags, 0, sizeof(response->flags));

	memset(&response->options, 0, sizeof(response->options));

	// DHCP magic cookie
	uint8_t magic_cookie[4] = {0x63, 0x82, 0x53, 0x63};
	memcpy(&response->options, &magic_cookie, sizeof(magic_cookie));
	int i = 4;

	// DHCP message type
	i += sprintf((char*)&response->options[i], "%c", DHCP_MESSAGE_TYPE);
	i += sprintf((char*)&response->options[i], "%c", 1);
	i += sprintf((char*)&response->options[i], "%c", type);

	// server ID
	struct sockaddr_in server_addr;
	if (inet_aton(config->server_ip, &server_addr.sin_addr) == 0) {
		return 1;
	}
	i += sprintf((char*)&response->options[i], "%c", SERVER_IDENTIFIER);
	i += sprintf((char*)&response->options[i], "%c", 4);
	memcpy(&response->options[i], &server_addr.sin_addr, sizeof(server_addr.sin_addr));
	i += sizeof(server_addr.sin_addr);

	// subnet mask
	struct sockaddr_in subnet_addr;
	if (inet_aton("255.255.255.0", &subnet_addr.sin_addr) == 0) {
		return 1;
	}
	i += sprintf((char*)&response->options[i], "%c", DHCP_OPTION_SUBNET);
	i += sprintf((char*)&response->options[i], "%c", 4);
	memcpy(&response->options[i], &subnet_addr.sin_addr, sizeof(subnet_addr.sin_addr));
	i += sizeof(subnet_addr.sin_addr);

	// end option
	response->options[i] = 0xff;

	struct sockaddr_in client_addr;
	if (inet_aton(config->client_ip, &client_addr.sin_addr) == 0) {
		return 1;
	}
	memcpy(&response->yiaddr, &client_addr.sin_addr, sizeof(response->yiaddr));

	return 0;
}


int dhcp_send_response(dhcp_config *config, dhcp_header *response, struct sockaddr_in *client_addr) {
	int len = sizeof(*response);

	client_addr->sin_addr.s_addr = inet_addr(config->client_ip);
	if (add_arp_entry(config, response->chaddr, response->hlen, response->yiaddr) != 0) {
		return 1;
	}
	if ((sendto(config->server_sock, response, len, 0, (struct sockaddr *)client_addr, sizeof(*client_addr))) < 0) {
		perror("Unable to send DHCP response");
		return 1;
	}

	return 0;
}

int dhcp_handle_discover(dhcp_config *config, dhcp_header *request, dhcp_header *response, struct sockaddr_in *client_addr) {
	if (dhcp_create_response(config, request, response, DHCP_OFFER) != 0) {
		return 1;
	}
	if (dhcp_send_response(config, response, client_addr) != 0)
		return 1;

	return 0;
}

int dhcp_handle_request(dhcp_config *config, dhcp_header *request, dhcp_header *response, struct sockaddr_in *client_addr) {
	if (dhcp_create_response(config, request, response, DHCP_ACK) != 0) {
		return 1;
	}
	if (dhcp_send_response(config, response, client_addr) != 0)
		return 1;

	return 0;
}

int dhcp_server_init(dhcp_config *config) {
	struct protoent *proto;

	if ((proto = getprotobyname("udp")) == 0) {
		perror("getprotobyname failed");
		goto ERROR;
	}

	if ((config->server_sock = socket(AF_INET, SOCK_DGRAM, proto->p_proto)) < 0) {
		perror("Unable to open UDP socket");
		goto ERROR;
	}

	int so_reuseaddr = 1;
	setsockopt(config->server_sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));

	memset(&config->server_addr, 0, sizeof(config->server_addr));
	config->server_addr.sin_family = AF_INET;
	config->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	config->server_addr.sin_port = htons(config->server_port);

	if (setsockopt(config->server_sock, SOL_SOCKET, SO_BINDTODEVICE, config->iface, strlen(config->iface)) != 0) {
		perror("Unable to bind to interface '%s'");
		goto ERROR;
	}

	if (bind(config->server_sock, (struct sockaddr*)&config->server_addr, sizeof(config->server_addr)) < 0)
	{
		perror("Unable to bind to UDP socket");
		goto ERROR;
	}

	return 0;

ERROR:
	if (config->server_sock >= 0)
		close(config->server_sock);
	return 1;
}

int dhcp_server_start(dhcp_config *config){
	printf("Trying to start server with parameters:\n");
	printf("Server IP addr: %s:%d, client IP addr: %s, interface: %s\n", config->server_ip, config->server_port,
		config->client_ip, config->iface);
	if (dhcp_server_init(config) != 0) {
		perror("Unable to initialize DHCP server");
		return 1;
	}
	printf("Server started!\n");

	for (;;) {
		dhcp_header request = {0};
		dhcp_header response = {0};
		struct sockaddr_in client_addr;
		socklen_t addr_len = sizeof(client_addr);
		memset(&client_addr, 0, addr_len);
		ssize_t recv_len;

		recv_len = recvfrom(config->server_sock, &request, sizeof(request), 0, (struct sockaddr *)&client_addr, &addr_len);
		if (recv_len < 0) {
			perror("Unable to receive from socket");
			close(config->server_sock);
			return 1;
		}

		if (request.op != BOOTREQUEST)
			continue;

		// Minimum size for a DHCP DISCOVER/REQUEST seems to be:
		// 243 bytes = DHCP Header (236 bytes) + DHCP magic (4) + type (1) + message (1) + 0xFF
		if (recv_len < 243)
			continue;
		int option_len = recv_len - DHCP_HEADER_SIZE;
		int idx = 4;
		int len;

		while (request.options[idx] != 0xFF && idx <= option_len) {
			if (request.options[idx] == DHCP_MESSAGE_TYPE) {
				len = (int)request.options[idx+1];
				if (request.options[idx+2] == DHCP_DISCOVER) {
					printf("Received DHCP DISCOVER from client: %s\n", mac_to_str(request.chaddr));
					dhcp_handle_discover(config, &request, &response, &client_addr);
					break;
				}
				if (request.options[idx+2] == DHCP_REQUEST) {
					printf("Received DHCP REQUEST from client: %s\n", mac_to_str(request.chaddr));
					dhcp_handle_request(config, &request, &response, &client_addr);
					break;
				}
			} else {
				len = (int)request.options[idx+1];
			}
			idx += len + 2;
		}
	}
	close(config->server_sock);
	return 0;
}

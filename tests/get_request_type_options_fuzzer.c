#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../server.h"

typedef int (*header_getter)(dhcp_header *h);

typedef struct {
	char *name;
	header_getter in_header;
	int expected;
} testcase;

int dhcp_get_request_type(dhcp_header *request, ssize_t request_len);

// Sets op code, message type, option magic and option message flag
void set_required_bits(uint8_t *h) {
	int o = DHCP_HEADER_SIZE;
	h[0] = BOOTREQUEST;
	h[1] = 1;  // ethernet
	h[o] = DHCP_OPTION_MAGIC >> 24;
	h[o+1] = (0xFF0000 & DHCP_OPTION_MAGIC) >> 16;
	h[o+2] = (0x00FF00 & DHCP_OPTION_MAGIC) >> 8;
	h[o+3] = 0x0000FF & DHCP_OPTION_MAGIC;
}


int get_fuzzy_options_header(dhcp_header *header) {
	uint8_t h[444] = {0};
	set_required_bits((uint8_t*)&h);
	memcpy(header, &h, sizeof(h));
	int rnd = open("/dev/urandom", O_RDONLY);
	read(rnd, header->options, sizeof(header->options));
	close(rnd);

	return sizeof(h);
}

int main(void) {
	int fails = 0;

	while (1) {
		dhcp_header h = {0};
		int size = get_fuzzy_options_header(&h);
		int out = dhcp_get_request_type(&h, size);
		// don't spin the CPU too much...
		usleep(10);
	}

	printf("dhcp_get_request_type: ok\n");

	return fails;
}

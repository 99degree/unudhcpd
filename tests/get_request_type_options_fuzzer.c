#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "../server.h"

typedef int (*message_getter)(dhcp_message *m);

typedef struct {
	char *name;
	message_getter in_message;
	int expected;
} testcase;

int dhcp_get_request_type(dhcp_message *request, ssize_t request_len);

// Sets op code, message type, option magic and option message flag
void set_required_bits(uint8_t *m) {
	int o = DHCP_HEADER_SIZE;
	m[0] = BOOTREQUEST;
	m[1] = 1;  // ethernet
	m[o] = DHCP_OPTION_MAGIC >> 24;
	m[o+1] = (0xFF0000 & DHCP_OPTION_MAGIC) >> 16;
	m[o+2] = (0x00FF00 & DHCP_OPTION_MAGIC) >> 8;
	m[o+3] = 0x0000FF & DHCP_OPTION_MAGIC;
}


int get_fuzzy_options_message(dhcp_message *message) {
	uint8_t m[444] = {0};
	set_required_bits((uint8_t*)&m);
	memcpy(message, &m, sizeof(m));
	int rnd = open("/dev/urandom", O_RDONLY);
	read(rnd, message->options, sizeof(message->options));
	close(rnd);

	return sizeof(m);
}

int main(void) {
	int fails = 0;

	while (1) {
		dhcp_message m = {0};
		int size = get_fuzzy_options_message(&m);
		dhcp_get_request_type(&m, size);
		// don't spin the CPU too much...
		usleep(10);
	}

	printf("dhcp_get_request_type: ok\n");

	return fails;
}

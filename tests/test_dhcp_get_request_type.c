#include <stdio.h>
#include <string.h>
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

int get_tiny_request_message(dhcp_message *message) {
	int o = DHCP_HEADER_SIZE;
	uint8_t m[DHCP_MESSAGE_SIZE_MIN] = {0};
	set_required_bits((uint8_t*)&m);
	m[o+4] = DHCP_MESSAGE_TYPE;
	m[o+5] = 1; 
	m[o+6] = DHCP_REQUEST; 
	m[o+7] = 0xFF;

	memcpy(message, &m, sizeof(m));
	return sizeof(m);
}

int get_tiny_discover_message(dhcp_message *message) {
	int o = DHCP_HEADER_SIZE;
	uint8_t m[DHCP_MESSAGE_SIZE_MIN] = {0};
	set_required_bits((uint8_t*)&m);
	m[o+4] = DHCP_MESSAGE_TYPE;
	m[o+5] = 1; 
	m[o+6] = DHCP_DISCOVER; 
	m[o+7] = 0xFF;

	memcpy(message, &m, sizeof(m));
	return sizeof(m);
}

int get_no_request_message_0(dhcp_message *message) {
	uint8_t m[432] = {0};
	set_required_bits((uint8_t*)&m);

	memcpy(message, &m, sizeof(m));
	return sizeof(m);
}

int get_no_request_message_2(dhcp_message *message) {
	uint8_t m[347] = {2};
	set_required_bits((uint8_t*)&m);

	memcpy(message, &m, sizeof(m));
	return sizeof(m);
}

// Before the end of a max-sized options buffer, add a len byte that tells the
// parser to read past the end of the buffer
int get_broken_message_overflow(dhcp_message *message) {
	int o = DHCP_HEADER_SIZE;
	uint8_t m[DHCP_MESSAGE_SIZE_MAX] = {0};
	set_required_bits((uint8_t*)&m);
	m[o+338] = DHCP_MESSAGE_TYPE; 
	m[o+339] = 1; 

	memcpy(message, &m, sizeof(m));
	return sizeof(m);
}

int get_large_discover_message(dhcp_message *message) {
	int o = DHCP_HEADER_SIZE;
	uint8_t m[DHCP_MESSAGE_SIZE_MAX] = {0};
	set_required_bits((uint8_t*)&m);
	m[o+336] = DHCP_MESSAGE_TYPE;
	m[o+337] = 1; 
	m[o+338] = DHCP_DISCOVER; 
	m[o+339] = 0xff; 

	memcpy(message, &m, sizeof(m));
	return sizeof(m);
}

int main(void) {
	int fails = 0;
	testcase tests[] = {
		{"DHCPREQUEST, small message", get_tiny_request_message, DHCP_REQUEST},
		{"DHCPDISCOVER, small message", get_tiny_discover_message, DHCP_DISCOVER},
		{"DHCPDISCOVER, large message", get_large_discover_message, DHCP_DISCOVER},
		{"Options all padding", get_no_request_message_0, -1},
		{"Options all 2's", get_no_request_message_2, -1},
		{"Broken message, overflow", get_broken_message_overflow, -1},
	};

	for (int i=0; i<(int)(sizeof(tests) / sizeof(tests[0])); i++) {
		testcase test = tests[i];
		dhcp_message m = {0};
		int size = test.in_message(&m);
		int out = dhcp_get_request_type(&m, size);
		if (test.expected != out) {
			fprintf(stderr, "dhcp_get_request_type (%s) failed: expected %d, got: %d\n",
					test.name, test.expected, out);
			fails++;
		}
	}

	if (fails == 0)
		printf("dhcp_get_request_type: ok\n");

	return fails;
}

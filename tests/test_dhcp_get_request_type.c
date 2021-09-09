#include <stdio.h>
#include <string.h>
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

int get_tiny_request_header(dhcp_header *header) {
	int o = DHCP_HEADER_SIZE;
	uint8_t h[243] = {0};
	set_required_bits((uint8_t*)&h);
	h[o+4] = DHCP_MESSAGE_TYPE;
	h[o+5] = 1; 
	h[o+6] = DHCP_REQUEST; 
	h[o+7] = 0xFF;

	memcpy(header, &h, sizeof(h));
	return sizeof(h);
}

int get_tiny_discover_header(dhcp_header *header) {
	int o = DHCP_HEADER_SIZE;
	uint8_t h[243] = {0};
	set_required_bits((uint8_t*)&h);
	h[o+4] = DHCP_MESSAGE_TYPE;
	h[o+5] = 1; 
	h[o+6] = DHCP_DISCOVER; 
	h[o+7] = 0xFF;

	memcpy(header, &h, sizeof(h));
	return sizeof(h);
}

int get_no_request_header_0(dhcp_header *header) {
	uint8_t h[432] = {0};
	set_required_bits((uint8_t*)&h);

	memcpy(header, &h, sizeof(h));
	return sizeof(h);
}

int get_no_request_header_2(dhcp_header *header) {
	uint8_t h[347] = {2};
	set_required_bits((uint8_t*)&h);

	memcpy(header, &h, sizeof(h));
	return sizeof(h);
}

// Before the end of a max-sized options buffer, add a len byte that tells the
// parser to read past the end of the buffer
int get_broken_header_overflow(dhcp_header *header) {
	int o = DHCP_HEADER_SIZE;
	uint8_t h[548] = {0};
	set_required_bits((uint8_t*)&h);
	h[o+310] = DHCP_MESSAGE_TYPE;
	h[o+311] = 1; 
	h[o+312] = DHCP_REQUEST; 

	memcpy(header, &h, sizeof(h));
	return sizeof(h);
}

int get_large_discover_header(dhcp_header *header) {
	int o = DHCP_HEADER_SIZE;
	uint8_t h[548] = {0};
	set_required_bits((uint8_t*)&h);
	h[o+308] = DHCP_MESSAGE_TYPE;
	h[o+309] = 1; 
	h[o+310] = DHCP_DISCOVER; 
	h[o+311] = 0xff; 

	memcpy(header, &h, sizeof(h));
	return sizeof(h);
}

int main(void) {
	int fails = 0;
	testcase tests[] = {
		{"DHCPREQUEST, small header", get_tiny_request_header, DHCP_REQUEST},
		{"DHCPDISCOVER, small header", get_tiny_discover_header, DHCP_DISCOVER},
		{"DHCPDISCOVER, large header", get_large_discover_header, DHCP_DISCOVER},
		{"Options all padding", get_no_request_header_0, -1},
		{"Options all 2's", get_no_request_header_2, -1},
		{"Broken header, overflow", get_broken_header_overflow, -1},
	};

	for (int i=0; i<(int)(sizeof(tests) / sizeof(tests[0])); i++) {
		testcase test = tests[i];
		dhcp_header h = {0};
		int size = test.in_header(&h);
		int out = dhcp_get_request_type(&h, size);
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

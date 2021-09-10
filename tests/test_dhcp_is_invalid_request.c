#include <stdio.h>
#include <string.h>
#include "../server.h"

typedef struct {
	uint8_t in_op;
	uint8_t in_htype;
	uint32_t in_option_magic;
	int in_size;
	int expected;
} testcase;

int dhcp_is_invalid_request(dhcp_message *request, ssize_t request_len);

int main(void) {
	int fails = 0;
	testcase tests[] = {
		// op, hw type, & size OK
		{1, 1, DHCP_OPTION_MAGIC, 270, 0},
		// invalid/unsupported op code
		{10, 1, DHCP_OPTION_MAGIC, 270, 1},
		{-127, 1, DHCP_OPTION_MAGIC, 270, 1},
		{'\0', 1, DHCP_OPTION_MAGIC, 270, 1},
		{'a', 1, DHCP_OPTION_MAGIC, 270, 1},
		// invalid hw types
		{1, 3, DHCP_OPTION_MAGIC, 270, 1},
		{1, 'b', DHCP_OPTION_MAGIC, 270, 1},
		// too small
		{1, 1, DHCP_OPTION_MAGIC, 42, 1},
		{1, 1, DHCP_OPTION_MAGIC, 242, 1},
		{1, 1, DHCP_OPTION_MAGIC, '\0', 1},
		// too big
		{1, 1, DHCP_OPTION_MAGIC, INT32_MAX, 1},
		{1, 1, DHCP_OPTION_MAGIC, 577, 1},
		// bad magic
		{1, 1, DHCP_OPTION_MAGIC - 1, 270, 1},
		{1, 1, 0x73825363, 270, 1},
		{1, 1, 0x825363, 270, 1},
	};

	for (int i=0; i<(int)(sizeof(tests) / sizeof(tests[0])); i++) {
		testcase test = tests[i];
		dhcp_message m = {0};
		m.op = test.in_op;
		m.htype = test.in_htype;
		m.options[0] = test.in_option_magic >> 24;
		m.options[1] = (0xFF0000 & test.in_option_magic) >> 16;
		m.options[2] = (0x00FF00 & test.in_option_magic) >> 8;
		m.options[3] = 0x0000FF & test.in_option_magic;

		int out = dhcp_is_invalid_request(&m, test.in_size);
		if (test.expected != out) {
			fprintf(stderr, "dhcp_is_invalid_request (op: %d, hwtype: %d, size: %d) failed: expected %d, got: %d\n",
					test.in_op, test.in_htype, test.in_size, test.expected, out);
			fails++;
		}
	}

	if (fails == 0)
		printf("dhcp_is_invalid_request: ok\n");

	return fails;
}

#include <stdio.h>
#include <string.h>
#include "../server.h"

typedef struct {
	uint8_t in_op;
	uint8_t in_htype;
	uint8_t in_option_magic[4];
	int in_size;
	int expected;
} testcase;

int dhcp_is_invalid_request(dhcp_message *request, ssize_t request_len);

int main(void) {
	int fails = 0;
	testcase tests[] = {
		// op, hw type, & size OK
		{1, 1, {0x63, 0x82, 0x53, 0x63}, 270, 0},
		// invalid/unsupported op code
		{10, 1, {0x63, 0x82, 0x53, 0x63}, 270, 1},
		{-127, 1, {0x63, 0x82, 0x53, 0x63}, 270, 1},
		{'\0', 1, {0x63, 0x82, 0x53, 0x63}, 270, 1},
		{'a', 1, {0x63, 0x82, 0x53, 0x63}, 270, 1},
		// invalid hw types
		{1, 3, {0x63, 0x82, 0x53, 0x63}, 270, 1},
		{1, 'b', {0x63, 0x82, 0x53, 0x63}, 270, 1},
		// too small
		{1, 1, {0x63, 0x82, 0x53, 0x63}, 42, 1},
		{1, 1, {0x63, 0x82, 0x53, 0x63}, 242, 1},
		{1, 1, {0x63, 0x82, 0x53, 0x63}, '\0', 1},
		// too big
		{1, 1, {0x63, 0x82, 0x53, 0x63}, INT32_MAX, 1},
		{1, 1, {0x63, 0x82, 0x53, 0x63}, 577, 1},
		// bad magic
		{1, 1, {0x63, 0x82, 0x53, 0x63}, 42, 1},
		{1, 1, {0x63, 0x82, 0x53, 0x63}, 242, 1},
		{1, 1, {0x63, 0x82, 0x53, 0x63}, '\0', 1},
	};

	for (int i=0; i<(int)(sizeof(tests) / sizeof(tests[0])); i++) {
		testcase test = tests[i];
		dhcp_message m = {0};
		m.op = test.in_op;
		m.htype = test.in_htype;
		memcpy(&m.options, &test.in_option_magic, sizeof(test.in_option_magic));

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

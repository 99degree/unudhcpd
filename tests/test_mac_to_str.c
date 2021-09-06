#include <stdio.h>
#include <string.h>
#include "../server.h"

typedef struct {
	uint8_t in[6];
	char *expected;
} testcase;

char *mac_to_str(uint8_t *mac);

int main(void) {
	int fails = 0;
	testcase tests[] = {
		{{1, 2, 3, 4, 5, 6}, "01:02:03:04:05:06"},
		{{0xab, 0xcd, 0xef, 0x01, 0x23, 0x45}, "ab:cd:ef:01:23:45"},
		{{0xab, 0xcd, 0xef, 0x01, 0x00, 0x00}, "ab:cd:ef:01:00:00"},
	};

	for (int i=0; i<(int)(sizeof(tests) / sizeof(tests[0])); i++) {
		testcase test = tests[i];
		char *out = mac_to_str(test.in);
		if (strncmp(out, test.expected, 20)) {
			fprintf(stderr, "mac_to_str failed: expected %s, got: %s\n",
					test.expected, out);
			fails++;
		}
	}

	if (fails == 0)
		printf("mac_to_str: ok\n");

	return fails;
}

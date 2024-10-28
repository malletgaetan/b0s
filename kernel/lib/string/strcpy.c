#include "kernel/lib/string/string.h"

char *strcpy(char *dst, char *src) {
	u64 i = 0;
	do {
		dst[i] = src[i];
		i++;
	} while (src[i] != '\0');
	return dst;
}

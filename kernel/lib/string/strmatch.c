#include "kernel/lib/string/string.h"

u64 strmatch(char *a, char *b) {
	u64 i = 0;
	while (a[i] && a[i] == b[i])
		i++;
	return i;
}
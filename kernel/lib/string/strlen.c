#include "kernel/types.h"

u64 strlen(char *str, u64 max)
{
	u64 i = 0;
	while (str[i] && max--) {
		i++;
	}
	return i;
}
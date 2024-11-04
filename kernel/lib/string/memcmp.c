#include "kernel/types.h"

u8 memcmp(u8 *a, u8 *b, u64 size)
{
	while (size--) {
		if (*(a + size) != *(b + size))
			return (1);
	}
	return (0);
}
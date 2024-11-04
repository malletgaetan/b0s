#include "kernel/lib/string/string.h"

void *memcpy(void *dst, const void *src, u64 n)
{
	u64 i = 0;

	if (!dst && !src)
		return (NULL);
	while (i < n) {
		((u8 *)(dst))[i] = ((u8 *)src)[i];
		i++;
	}
	return (dst);
}
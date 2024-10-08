#ifndef STRING_H
# define STRING_H
# include "kernel/types.h"

u64 	strlen(char *str, u64 max);
void	*memset(void *, u8, u64);
u8		memcmp(u8 *a, u8 *b, u64 size);
void 	*memcpy(void *dst, const void *src, u64 n);

#endif
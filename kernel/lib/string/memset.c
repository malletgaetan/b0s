#include "kernel/lib/string/string.h"

void *memset(void *ptr, u8 val, u64 size) {
	for (u64 i = 0; i < size; i++) {
		((u8 *)ptr)[i] = val;
	}
	return ptr;
}
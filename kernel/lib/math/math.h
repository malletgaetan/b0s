#ifndef MATH_H
#define MATH_H
#include "kernel/types.h"

#define KB 0x400
#define MB 0x100000
#define GB 0x40000000

#define MAX(a, b) (((u64)a) > ((u64)b) ? (u64)(a) : (u64)(b))
#define MIN(a, b) (((u64)a) < ((u64)b) ? (u64)(a) : (u64)(b))
#define ROUNDUP(nb, sz) (((u64)nb + (u64)sz) & ~((u64)sz - 1))
#define IS_ALIGNED(a, b) (((u64)(a) & ((b) - 1)) == 0)
#define BYTE2MB(x) ((x) / (1024 * 1024))
#define BYTE2KB(x) ((x) / 1024)

#endif
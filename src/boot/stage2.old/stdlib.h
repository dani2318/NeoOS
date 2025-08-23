#pragma once
#include <stdint.h>
#include <stddef.h>
#include "memory.h"

void qsort(void* base, size_t num, size_t size, int (*compare)(const void* a, const void* b));
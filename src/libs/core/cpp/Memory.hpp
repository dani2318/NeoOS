#pragma once
#include <stdint.h>
#include <stddef.h>
#include <core/Defs.hpp>
#include <core/Debug.hpp>
#include <boot/bootparams.hpp>

EXPORT void* ASMCALL memcpy(void* dest, void* src, size_t count);

namespace Memory
{
    constexpr auto Copy = memcpy;
} // namespace Memory

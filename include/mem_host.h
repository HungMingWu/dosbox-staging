/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  Copyright (C) 2020-2021  The DOSBox Staging Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DOSBOX_MEM_HOST_H
#define DOSBOX_MEM_HOST_H

/* These functions read and write fixed-width unsigned integers to/from
 * byte-arrays using DOS/little-endian byte ordering.
 *
 * Values returned or passed to these functions are expected to be integers
 * using native endianness representation for the host machine.
 *
 * They are safe to use even when byte-array address is not aligned according
 * to desired integer width.
 */

#include <cstdint>

#include "byteorder.h"
#include "mem_unaligned.h"

/* Use host_read functions to replace endian branching and byte swapping code,
 * such as:
 *
 *   #ifdef WORDS_BIGENDIAN
 *   auto x = byteswap(*(uint16_t*)(arr));
 *   #else
 *   auto x = *(uint16_t*)(arr);
 *   #endif
 */

template <typename T>
constexpr T host_read(const uint8_t *arr) noexcept
{
	if constexpr (sizeof(T) == 1) {
		// Read a single-byte value; provided for backwards-compatibility only.
		return *arr;
	} else if constexpr (sizeof(T) == 2) {
		// Read a 16-bit word from 8-bit DOS/little-endian byte-ordered memory.
		return le16_to_host(read_unaligned_uint16(arr));
	} else if constexpr (sizeof(T) == 4) {
		// Read a 32-bit double-word from 8-bit DOS/little-endian byte-ordered memory.
		return le32_to_host(read_unaligned_uint32(arr));
	} else {
		// Read a 64-bit quad-word from 8-bit DOS/little-endian byte-ordered memory.
		return le64_to_host(read_unaligned_uint64(arr));
	}
	return T{};
}

/* Use host_read_at functions to replace endian branching and byte swapping
 * code such as:
 *
 *   #ifdef WORDS_BIGENDIAN
 *   auto x = byteswap(((uint16_t*)arr)[idx]);
 *   #else
 *   auto x = ((uint16_t*)arr)[idx];
 *   #endif
 */

template <typename T>
constexpr T host_read_at(const uint8_t* arr, const uintptr_t idx) noexcept
{
	return host_read<T>(arr + idx * sizeof(T));
}

/* Use host_write functions to replace endian branching and byte swapping
 * code such as:
 *
 *   #ifdef WORDS_BIGENDIAN
 *   *(uint16_t*)arr = byteswap((uint16_t)val);
 *   #else
 *   *(uint16_t*)arr = (uint16_t)val;
 *   #endif
 */

template <typename T>
constexpr void host_write(uint8_t *arr, const T val) noexcept
{
	if constexpr (sizeof(T) == 1) {
		// Write a single-byte value; provided for backwards-compatibility only.
		*arr = val;
	} else if constexpr (sizeof(T) == 2) {
		// Write a 16-bit word to 8-bit memory using DOS/little-endian byte-ordering.
		write_unaligned_uint16(arr, host_to_le16(val));
	} else if constexpr (sizeof(T) == 4) {
		// Write a 32-bit double-word to 8-bit memory using DOS/little-endian byte-ordering.
		write_unaligned_uint32(arr, host_to_le32(val));
	} else {
		// Write a 64-bit quad-word to 8-bit memory using DOS/little-endian byte-ordering.
		write_unaligned_uint64(arr, host_to_le64(val));
	}
}

/* Use host_write_at functions to replace endian branching and byte swapping
 * code such as:
 *
 *   #ifdef WORDS_BIGENDIAN
 *   ((uint16_t*)arr)[idx] = byteswap((uint16_t)val);
 *   #else
 *   ((uint16_t*)arr)[idx] = (uint16_t)val;
 *   #endif
 */

template <typename T>
constexpr void host_write_at(uint8_t *arr, const uintptr_t idx, const T val) noexcept
{
	return host_write<T>(arr + idx * sizeof(T), val);
}

/* Use host_add functions to replace endian branching and byte swapping
 * code such as:
 *
 *   #ifdef WORDS_BIGENDIAN
 *   *(uint16_t*)arr += byteswap((uint16_t)val);
 *   #else
 *   *(uint16_t*)arr += val;
 *   #endif
 */

template <typename T>
static inline void host_add(uint8_t *arr, const T val) noexcept
{
	host_write<T>(arr, host_read<T>(arr) + val);
}

/* Use host_inc functions to replace endian branching and byte swapping
 * code such as:
 *
 *   #ifdef WORDS_BIGENDIAN
 *   *(uint16_t*)arr += byteswap((uint16_t)1);
 *   #else
 *   *(uint16_t*)arr += 1;
 *   #endif
 */

template <typename T>
static inline void host_inc(uint8_t *arr) noexcept
{
	host_write<T>(arr, host_read<T>(arr) + 1);
}

#endif

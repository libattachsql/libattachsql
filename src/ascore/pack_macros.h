/* vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 *
 * Packing/unpacking macros from
 * Drizzle Client & Protocol Library
 *
 * Copyright (C) 2008-2013 Drizzle Developer Group
 * Copyright (C) 2011 Brian Aker (brian@tangent.org)
 * Copyright (C) 2008 Eric Day (eday@oddments.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 *     * The names of its contributors may not be used to endorse or
 * promote products derived from this software without specific prior
 * written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ascore_pack_int2(__buffer, __int) do { \
  (__buffer)[0]= (uint8_t)((__int) & 0xFF); \
  (__buffer)[1]= (uint8_t)(((__int) >> 8) & 0xFF); } while (0)
#define ascore_pack_int3(__buffer, __int) do { \
  (__buffer)[0]= (uint8_t)((__int) & 0xFF); \
  (__buffer)[1]= (uint8_t)(((__int) >> 8) & 0xFF); \
  (__buffer)[2]= (uint8_t)(((__int) >> 16) & 0xFF); } while (0)
#define ascore_pack_int4(__buffer, __int) do { \
  (__buffer)[0]= (uint8_t)((__int) & 0xFF); \
  (__buffer)[1]= (uint8_t)(((__int) >> 8) & 0xFF); \
  (__buffer)[2]= (uint8_t)(((__int) >> 16) & 0xFF); \
  (__buffer)[3]= (uint8_t)(((__int) >> 24) & 0xFF); } while (0)
#define ascore_pack_int8(__buffer, __int) do { \
  (__buffer)[0]= (uint8_t)((__int) & 0xFF); \
  (__buffer)[1]= (uint8_t)(((__int) >> 8) & 0xFF); \
  (__buffer)[2]= (uint8_t)(((__int) >> 16) & 0xFF); \
  (__buffer)[3]= (uint8_t)(((__int) >> 24) & 0xFF); \
  (__buffer)[4]= (uint8_t)(((__int) >> 32) & 0xFF); \
  (__buffer)[5]= (uint8_t)(((__int) >> 40) & 0xFF); \
  (__buffer)[6]= (uint8_t)(((__int) >> 48) & 0xFF); \
  (__buffer)[7]= (uint8_t)(((__int) >> 56) & 0xFF); } while (0)

#define ascore_unpack_int2(__buffer)               \
            ((((uint8_t *)__buffer)[0]) |         \
  ((uint16_t)(((uint8_t *)__buffer)[1]) << 8))
#define ascore_unpack_int3(__buffer)               \
             (((uint8_t *)__buffer)[0] |          \
  ((uint32_t)(((uint8_t *)__buffer)[1]) << 8) |   \
  ((uint32_t)(((uint8_t *)__buffer)[2]) << 16))
#define ascore_unpack_int4(__buffer)               \
             (((uint8_t *)__buffer)[0] |          \
  ((uint32_t)(((uint8_t *)__buffer)[1]) << 8) |   \
  ((uint32_t)(((uint8_t *)__buffer)[2]) << 16) |  \
  ((uint32_t)(((uint8_t *)__buffer)[3]) << 24))
#define ascore_unpack_int8(__buffer)               \
  (ascore_unpack_int4(__buffer) |                  \
  ((uint64_t)ascore_unpack_int4(((uint8_t *)__buffer)+4) << 32))

#define ascore_mb_char(__c) (((__c) & 0x80) != 0)
#define ascore_mb_length(__c) \
  ((uint32_t)(__c) <= 0x7f ? 1 : \
  ((uint32_t)(__c) <= 0x7ff ? 2 : \
  ((uint32_t)(__c) <= 0xd7ff ? 3 : \
  ((uint32_t)(__c) <= 0xdfff || (uint32_t)(__c) > 0x10ffff ? 0 : \
  ((uint32_t)(__c) <= 0xffff ? 3 : 4)))))

#ifdef __cplusplus
}
#endif

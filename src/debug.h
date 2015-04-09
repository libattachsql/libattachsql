/* vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * Copyright 2014 Hewlett-Packard Development Company, L.P.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain 
 * a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 */

#pragma once

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if DEBUG
#define asdebug(MSG, ...) fprintf(stderr, "[libattachsql] %s:%d " MSG "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define asdebug_hex(DATA, LEN) do { \
  size_t hex_it; \
  fprintf(stderr, "[libattachsql] %s:%d packet hex: ", __FILE__, __LINE__); \
  for (hex_it = 0; hex_it < LEN ; hex_it++) \
  { \
    fprintf(stderr, "%02X ", (unsigned char)DATA[hex_it]); \
  } \
  fprintf(stderr, "\n"); \
  fprintf(stderr, "[libattachsql] %s:%d printable packet data: ", __FILE__, __LINE__); \
  for (hex_it = 0; hex_it < LEN ; hex_it++) \
  { \
    if (((unsigned char)DATA[hex_it] < 0x32) or (((unsigned char)DATA[hex_it] > 0x7e))) \
    { \
      fprintf(stderr, "."); \
    } \
    else \
    { \
      fprintf(stderr, "%c", (unsigned char)DATA[hex_it]); \
    } \
  } \
  fprintf(stderr, "\n"); \
} while(0)
#else
#define asdebug(MSG, ...)
#define asdebug_hex(DATA, LEN)
#endif

#ifdef __cplusplus
}
#endif

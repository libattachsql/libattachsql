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

#include <sys/types.h>
#include <stdint.h>
#include "return.h"

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#endif

#define ASCORE_DEFAULT_BUFFER_SIZE 1024*1024

struct buffer_st
{
  char *buffer;
  size_t buffer_size;
  size_t buffer_used;
  char *buffer_write_ptr;
  char *buffer_read_ptr;
  char *packet_end_ptr;

  buffer_st() :
    buffer(NULL),
    buffer_size(0),
    buffer_used(0),
    buffer_write_ptr(NULL),
    buffer_read_ptr(NULL),
    packet_end_ptr(NULL)
  { }
};

buffer_st *ascore_buffer_create();
void ascore_buffer_free(buffer_st *buffer);
size_t ascore_buffer_get_available(buffer_st *buffer);
asret_t ascore_buffer_increase(buffer_st *buffer);
void ascore_buffer_move_write_ptr(buffer_st *buffer, size_t len);
size_t ascore_buffer_unread_data(buffer_st *buffer);
void ascore_buffer_packet_read_end(buffer_st *buffer);

#ifdef __cplusplus
}
#endif

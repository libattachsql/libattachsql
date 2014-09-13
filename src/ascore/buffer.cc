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

#include "config.h"
#include "common.h"
#include "buffer.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

buffer_st *ascore_buffer_create()
{
  buffer_st *buffer;

  buffer= new (std::nothrow) buffer_st;
  if (buffer == NULL)
  {
    return NULL;
  }

  buffer->buffer= (char*)malloc(ASCORE_DEFAULT_BUFFER_SIZE);
  if (buffer->buffer == NULL)
  {
    delete buffer;
    return NULL;
  }

  buffer->buffer_read_ptr= buffer->buffer;
  buffer->buffer_write_ptr= buffer->buffer;
  buffer->buffer_size= ASCORE_DEFAULT_BUFFER_SIZE;

  return buffer;
}

void ascore_buffer_free(buffer_st *buffer)
{
  free(buffer->buffer);
  delete buffer;
}

size_t ascore_buffer_get_available(buffer_st *buffer)
{
  if (buffer == NULL)
  {
    return 0;
  }
  return buffer->buffer_size - ((size_t)(buffer->buffer_read_ptr - buffer->buffer) + buffer->buffer_used);
}

asret_t ascore_buffer_increase(buffer_st *buffer)
{
  if (buffer == NULL)
  {
    return ASRET_PARAMETER_ERROR;
  }

  size_t buffer_available= ascore_buffer_get_available(buffer);

  /* if the we have lots of stale data just shift
   * algorithm for this at the moment is if only half the buffer is available
   * move it
   */
  if (buffer_available + buffer->buffer_used < (buffer->buffer_size / 2))
  {
    memmove(buffer->buffer, buffer->buffer_read_ptr, buffer->buffer_used);
    buffer->buffer_write_ptr= buffer->buffer + buffer->buffer_used;
  }
  else
  {
    size_t buffer_write_size= buffer->buffer_write_ptr - buffer->buffer;
    size_t buffer_read_size= buffer->buffer_read_ptr - buffer->buffer;
    size_t packet_end_size;
    if (buffer->packet_end_ptr != NULL)
    {
      packet_end_size= buffer->packet_end_ptr - buffer->buffer;
    }
    else
    {
      packet_end_size= 0;
    }
    size_t new_size= buffer->buffer_size * 2;
    char *realloc_buffer= (char*)realloc(buffer->buffer, new_size);
    if (realloc_buffer == NULL)
    {
      return ASRET_OUT_OF_MEMORY_ERROR;
    }
    buffer->buffer_size= new_size;
    buffer->buffer= realloc_buffer;
    /* Move pointers because buffer may have moved in RAM */
    buffer->buffer_write_ptr= realloc_buffer + buffer_write_size;
    buffer->buffer_read_ptr= realloc_buffer + buffer_read_size;
    buffer->packet_end_ptr= realloc_buffer + packet_end_size;
  }

  return ASRET_OK;
}

/* Moves the write pointer and returns the amount of unread data in the buffer */
void ascore_buffer_move_write_ptr(buffer_st *buffer, size_t len)
{
  buffer->buffer_write_ptr+= len;
  buffer->buffer_used+= len;
}

size_t ascore_buffer_unread_data(buffer_st *buffer)
{
  return (size_t)(buffer->buffer_write_ptr - buffer->buffer_read_ptr);
}

void ascore_buffer_packet_read_end(buffer_st *buffer)
{
  // If the buffer is now empty, reset it.  Otherwise make sure the read ptr
  // points to the end of the packet.  Should maybe move this to buffer.cc
  if (buffer->packet_end_ptr == buffer->buffer_write_ptr)
  {
    asdebug("Truncating buffer");
    buffer->buffer_read_ptr= buffer->buffer;
    buffer->buffer_write_ptr= buffer->buffer;
    buffer->buffer_used= 0;
    buffer->packet_end_ptr= NULL;
  }
  else
  {
    buffer->buffer_read_ptr= buffer->packet_end_ptr;
  }
}

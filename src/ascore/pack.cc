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
#include "pack.h"
#include "pack_macros.h"

/* unpack a variable length int from a char buffer.  Returns the int data along
 * with the number of bytes taken from the buffer in 'bytes'.  If there is an
 * error or NULL it is reflected in 'status'.
 * Requires 'bytes' to be pre-allocated (status can be NULL to not use it).
 */
uint64_t ascore_unpack_length(char *buffer, uint8_t *bytes, ascore_pack_status_t *status)
{
  ascore_pack_status_t unused_status;
  if (status == NULL)
  {
    status= &unused_status;
  }
  if ((bytes == NULL) || (buffer == NULL))
  {
    *status= ASCORE_PACK_INVALID_ARGUMENT;
  }
  *status= ASCORE_PACK_OK;

  if ((unsigned char)buffer[0] < 0xfb)
  {
    *bytes= 1;
    return (uint64_t) buffer[0];
  }
  else if ((unsigned char)buffer[0] == 0xfb)
  {
    *bytes= 1;
    *status= ASCORE_PACK_NULL;
    return 0;
  }
  else if ((unsigned char)buffer[0] == 0xfc)
  {
    *bytes= 3;
    return ascore_unpack_int2(buffer + 1);
  }
  else if ((unsigned char)buffer[0] == 0xfd)
  {
    *bytes= 4;
    return ascore_unpack_int3(buffer + 1);
  }
  else
  {
    *bytes= 9;
    return ascore_unpack_int8(buffer + 1);
  }
  // If you get here, your compiler doesn't love you
}

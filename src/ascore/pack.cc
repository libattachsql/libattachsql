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
uint64_t attachsql_unpack_length(char *buffer, uint8_t *bytes, attachsql_pack_status_t *status)
{
  attachsql_pack_status_t unused_status;
  if (status == NULL)
  {
    status= &unused_status;
  }
  if ((bytes == NULL) || (buffer == NULL))
  {
    *status= ATTACHSQL_PACK_INVALID_ARGUMENT;
    return 0;
  }
  *status= ATTACHSQL_PACK_OK;

  if ((unsigned char)buffer[0] < 0xfb)
  {
    *bytes= 1;
    return (uint64_t) buffer[0];
  }
  else if ((unsigned char)buffer[0] == 0xfb)
  {
    *bytes= 1;
    *status= ATTACHSQL_PACK_NULL;
    return 0;
  }
  else if ((unsigned char)buffer[0] == 0xfc)
  {
    *bytes= 3;
    return attachsql_unpack_int2(buffer + 1);
  }
  else if ((unsigned char)buffer[0] == 0xfd)
  {
    *bytes= 4;
    return attachsql_unpack_int3(buffer + 1);
  }
  else
  {
    *bytes= 9;
    return attachsql_unpack_int8(buffer + 1);
  }
  // If you get here, your compiler doesn't love you
}

char *attachsql_pack_data(char *buffer, size_t length, char *data)
{
  buffer= attachsql_pack_length(buffer, length);
  if (length > 0)
  {
    memcpy(buffer, data, length);
    buffer+= length;
  }

  return buffer;
}

char *attachsql_pack_length(char *buffer, size_t length)
{
  if (length <= 0xfa)
  {
    buffer[0]= (uint8_t)length;
    buffer++;
  }
  else if (length <= UINT16_MAX)
  {
    buffer[0]= 0xfc;
    buffer++;
    attachsql_pack_int2(buffer, length);
    buffer+= 2;
  }
  else if (length <= 0xffffff)
  {
    buffer[0]= 0xfd;
    buffer++;
    attachsql_pack_int3(buffer, length);
    buffer+= 3;
  }
  else
  {
    buffer[0]= 0xfe;
    buffer++;
    attachsql_pack_int8(buffer, (uint64_t)length);
    buffer+= 8;
  }

  return buffer;
}

char *attachsql_pack_datetime(char *buffer, attachsql_datetime_st *datetime, bool date_only)
{
  uint8_t length= 0;

  if (not date_only)
  {
    if (datetime->microsecond > 0)
    {
      attachsql_pack_int4(buffer+8, datetime->microsecond);
      length= 11;
    }

    if ((length > 0) or
        (datetime->hour > 0) or
        (datetime->minute > 0) or
        (datetime->second > 0))
    {
      buffer[5]= (char) datetime->hour;
      buffer[6]= (char) datetime->minute;
      buffer[7]= (char) datetime->second;
      if (length == 0)
      {
        length= 7;
      }
    }
  }

  if ((length > 0) or
      (datetime->year > 0) or
      (datetime->month > 0) or
      (datetime->day > 0))
  {
    attachsql_pack_int2(buffer+1, datetime->year);
    buffer[3]= (char) datetime->month;
    buffer[4]= (char) datetime->day;
    if (length == 0)
    {
      length= 4;
    }
  }

  buffer[0]= (char) length;

  return (buffer + length + 1);
}

void attachsql_unpack_datetime(char *buffer, size_t length, attachsql_datetime_st *datetime)
{
  if (length)
  {
    datetime->is_negative= false;
    datetime->year= attachsql_unpack_int2(buffer);
    datetime->month= buffer[2];
    datetime->day= buffer[3];
    if (length > 4)
    {
      datetime->hour= buffer[4];
      datetime->minute= buffer[5];
      datetime->second= buffer[6];
      if (length > 7)
      {
        datetime->microsecond= attachsql_unpack_int4(&buffer[7]);
      }
    }
  }
}

void attachsql_unpack_time(char *buffer, size_t length, attachsql_datetime_st *datetime)
{
  if (length)
  {
    datetime->is_negative= buffer[0];
    datetime->day= attachsql_unpack_int4(&buffer[1]);
    datetime->hour= buffer[5];
    datetime->minute= buffer[6];
    datetime->second= buffer[7];
    if (length > 8)
    {
      datetime->microsecond= attachsql_unpack_int4(&buffer[8]);
    }
  }
}

char *attachsql_pack_time(char *buffer, attachsql_datetime_st *time)
{
  uint8_t length= 0;

  if (time->microsecond > 0)
  {
    attachsql_pack_int4(buffer+9, time->microsecond);
    length= 12;
  }

  if ((length > 0) or
      (time->day > 0) or
      (time->minute > 0) or
      (time->second > 0))
  {
    buffer[1]= time->is_negative;
    attachsql_pack_int4(buffer+2, time->day);
    buffer[6]= (char) time->hour;
    buffer[7]= (char) time->minute;
    buffer[8]= (char) time->second;
    if (length == 0)
    {
      length= 8;
    }
  }

  buffer[0]= (char) length;
  return (buffer + length + 1);
}

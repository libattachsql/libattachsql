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

#include <stdint.h>
#include <string.h>
#include "constants.h"
#include "structs.h"

#ifdef __cplusplus
extern "C" {
#endif

enum attachsql_pack_status_t
{
  ATTACHSQL_PACK_OK,
  ATTACHSQL_PACK_INVALID_ARGUMENT,
  ATTACHSQL_PACK_NULL
};

uint64_t attachsql_unpack_length(char *buffer, uint8_t *bytes, attachsql_pack_status_t *status);

char *attachsql_pack_data(char *buffer, size_t length, char *data);

char *attachsql_pack_length(char *buffer, size_t length);

char *attachsql_pack_datetime(char *buffer, attachsql_datetime_st *datetime, bool date_only);

char *attachsql_pack_time(char *buffer, attachsql_datetime_st *datetime);

void attachsql_unpack_time(char *buffer, size_t length, attachsql_datetime_st *datetime);

void attachsql_unpack_datetime(char *buffer, size_t length, attachsql_datetime_st *datetime);

#ifdef __cplusplus
}
#endif

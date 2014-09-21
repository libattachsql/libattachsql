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

enum ascore_pack_status_t
{
  ASCORE_PACK_OK,
  ASCORE_PACK_INVALID_ARGUMENT,
  ASCORE_PACK_NULL
};

uint64_t ascore_unpack_length(char *buffer, uint8_t *bytes, ascore_pack_status_t *status);

char *ascore_pack_data(char *buffer, size_t length, char *data);

char *ascore_pack_length(char *buffer, size_t length);

char *ascore_pack_datetime(char *buffer, ascore_datetime_st *datetime, bool date_only);

char *ascore_pack_time(char *buffer, ascore_datetime_st *datetime);

void ascore_unpack_time(char *buffer, size_t length, ascore_datetime_st *datetime);

void ascore_unpack_datetime(char *buffer, size_t length, ascore_datetime_st *datetime);

#ifdef __cplusplus
}
#endif

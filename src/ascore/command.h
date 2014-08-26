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
#include "common.h"
#include "connect.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_ZLIB
ascore_command_status_t ascore_command_send_compressed(ascon_st *con, ascore_command_t command, char *data, size_t length);
#endif

ascore_command_status_t ascore_command_send(ascon_st *con, ascore_command_t command, char *data, size_t length);

ascore_command_status_t ascore_get_next_row(ascon_st *con);

bool ascore_command_next_result(ascon_st *con);

void ascore_command_free(ascon_st *con);

#ifdef __cplusplus
}
#endif

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

#include "src/ascore/ascore.h"

#ifdef __cplusplus
extern "C" {
#endif

attachsql_error_st *attachsql_statement_set_param(attachsql_connect_t *con, ascore_column_type_t type, uint16_t param, size_t length, const void *value, bool is_unsigned);

#ifdef __cplusplus
}
#endif


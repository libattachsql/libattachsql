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

#include <float.h>
#include "src/ascore/ascore.h"

#ifdef __cplusplus
extern "C" {
#endif

// Float and Double lengths from: http://stackoverflow.com/questions/1701055/what-is-the-maximum-length-in-chars-needed-to-represent-any-double-value

#define FLOAT_MAX_LEN 3 + FLT_MANT_DIG - FLT_MIN_EXP
#define DOUBLE_MAX_LEN 3 + DBL_MANT_DIG - DBL_MIN_EXP

size_t attachsql_query_escape_data(char *buffer, char *data, size_t length);

attachsql_return_t attachsql_query_row_buffer(attachsql_connect_t *con, attachsql_error_st **error);
#ifdef __cplusplus
}
#endif


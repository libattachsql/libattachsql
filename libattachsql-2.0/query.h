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

#ifdef __cplusplus
extern "C" {
#endif

ASQL_API
bool attachsql_query(attachsql_connect_t *con, size_t length, const char *statement, uint16_t parameter_count, attachsql_query_parameter_st *parameters, attachsql_error_t **error);

ASQL_API
void attachsql_query_close(attachsql_connect_t *con);

ASQL_API
uint16_t attachsql_query_column_count(attachsql_connect_t *con);

ASQL_API
attachsql_query_column_st *attachsql_query_column_get(attachsql_connect_t *con, uint16_t column);

ASQL_API
attachsql_query_row_st *attachsql_query_row_get(attachsql_connect_t *con, attachsql_error_t **error);

ASQL_API
void attachsql_query_row_next(attachsql_connect_t *con);

ASQL_API
uint64_t attachsql_connection_last_insert_id(attachsql_connect_t *con);

ASQL_API
uint64_t attachsql_query_affected_rows(attachsql_connect_t *con);

ASQL_API
const char *attachsql_query_info(attachsql_connect_t *con);

ASQL_API
uint32_t attachsql_query_warning_count(attachsql_connect_t *con);

ASQL_API
attachsql_return_t attachsql_query_next_result(attachsql_connect_t *con);

ASQL_API
bool attachsql_query_buffer_rows(attachsql_connect_t *con, bool enable);

ASQL_API
uint64_t attachsql_query_row_count(attachsql_connect_t *con);

ASQL_API
attachsql_query_row_st *attachsql_query_buffer_row_get(attachsql_connect_t *con);

ASQL_API
attachsql_query_row_st *attachsql_query_row_get_offset(attachsql_connect_t *con, uint64_t row_number);

#ifdef __cplusplus
}
#endif


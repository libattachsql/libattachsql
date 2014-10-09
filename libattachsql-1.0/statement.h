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
bool attachsql_statement_prepare(attachsql_connect_t *con, size_t length, const char *statement, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_execute(attachsql_connect_t *con, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_reset(attachsql_connect_t *con, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_send_long_data(attachsql_connect_t *con, uint16_t param, size_t length, char *data, attachsql_error_t **error);

ASQL_API
uint16_t attachsql_statement_get_param_count(attachsql_connect_t *con);

ASQL_API
bool attachsql_statement_set_int(attachsql_connect_t *con, uint16_t param, int32_t value, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_set_unsigned_int(attachsql_connect_t *con, uint16_t param, uint32_t value, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_set_bigint(attachsql_connect_t *con, uint16_t param, int64_t value, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_set_unsigned_bigint(attachsql_connect_t *con, uint16_t param, uint64_t value, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_set_float(attachsql_connect_t *con, uint16_t param, float value, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_set_double(attachsql_connect_t *con, uint16_t param, double value, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_set_string(attachsql_connect_t *con, uint16_t param, size_t length, const char *value, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_set_binary(attachsql_connect_t *con, uint16_t param, size_t length, const char *value, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_set_null(attachsql_connect_t *con, uint16_t param, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_set_datetime(attachsql_connect_t *con, uint16_t param, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, uint32_t microsecond, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_set_time(attachsql_connect_t *con, uint16_t param, uint8_t hour, uint8_t minute, uint8_t second, uint32_t microsecond, bool is_negative, attachsql_error_t **error);

ASQL_API
bool attachsql_statement_row_get(attachsql_connect_t *con, attachsql_error_t **error);

ASQL_API
int32_t attachsql_statement_get_int(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error);

ASQL_API
uint32_t attachsql_statement_get_int_unsigned(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error);

ASQL_API
int64_t attachsql_statement_get_bigint(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error);

ASQL_API
uint64_t attachsql_statement_get_bigint_unsigned(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error);

ASQL_API
double attachsql_statement_get_double(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error);

ASQL_API
float attachsql_statement_get_float(attachsql_connect_t *con, uint16_t column, attachsql_error_t **error);

ASQL_API
char *attachsql_statement_get_char(attachsql_connect_t *con, uint16_t column, size_t *length, attachsql_error_t **error);

ASQL_API
void attachsql_statement_close(attachsql_connect_t *con);

ASQL_API
attachsql_column_type_t attachsql_statement_get_column_type(attachsql_connect_t *con, uint16_t column);

ASQL_API
void attachsql_statement_row_next(attachsql_connect_t *con);

ASQL_API
uint16_t attachsql_statement_get_column_count(attachsql_connect_t *con);

#ifdef __cplusplus
}
#endif


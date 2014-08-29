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

enum attachsql_column_type_t
{
  ATTACHSQL_COLUMN_TYPE_DECIMAL=     0x00,
  ATTACHSQL_COLUMN_TYPE_TINY=        0x01,
  ATTACHSQL_COLUMN_TYPE_SHORT=       0x02,
  ATTACHSQL_COLUMN_TYPE_LONG=        0x03,
  ATTACHSQL_COLUMN_TYPE_FLOAT=       0x04,
  ATTACHSQL_COLUMN_TYPE_DOUBLE=      0x05,
  ATTACHSQL_COLUMN_TYPE_NULL=        0x06,
  ATTACHSQL_COLUMN_TYPE_TIMESTAMP=   0x07,
  ATTACHSQL_COLUMN_TYPE_LONGLONG=    0x08,
  ATTACHSQL_COLUMN_TYPE_INT24=       0x09,
  ATTACHSQL_COLUMN_TYPE_DATE=        0x0a,
  ATTACHSQL_COLUMN_TYPE_TIME=        0x0b,
  ATTACHSQL_COLUMN_TYPE_DATETIME=    0x0c,
  ATTACHSQL_COLUMN_TYPE_YEAR=        0x0d,
  ATTACHSQL_COLUMN_TYPE_NEWDATE=     0x0e, // Internal only
  ATTACHSQL_COLUMN_TYPE_VARCHAR=     0x0f, // Internal only
  ATTACHSQL_COLUMN_TYPE_BIT=         0x10,
  ATTACHSQL_COLUMN_TYPE_TIMESTAMP2=  0x11, // Internal only
  ATTACHSQL_COLUMN_TYPE_DATETIME2=   0x12, // Internal only
  ATTACHSQL_COLUMN_TYPE_TIME2=       0x13, // Internal only
  ATTACHSQL_COLUMN_TYPE_NEWDECIMAL=  0xf6,
  ATTACHSQL_COLUMN_TYPE_ENUM=        0xf7,
  ATTACHSQL_COLUMN_TYPE_SET=         0xf8,
  ATTACHSQL_COLUMN_TYPE_TINY_BLOB=   0xf9, // Internal only
  ATTACHSQL_COLUMN_TYPE_MEDIUM_BLOB= 0xfa, // Internal only
  ATTACHSQL_COLUMN_TYPE_LONG_BLOB=   0xfb, // Internal only
  ATTACHSQL_COLUMN_TYPE_BLOB=        0xfc,
  ATTACHSQL_COLUMN_TYPE_VARSTRING=   0xfd,
  ATTACHSQL_COLUMN_TYPE_STRING=      0xfe,
  ATTACHSQL_COLUMN_TYPE_GEOMETRY=    0xff
};

typedef enum attachsql_column_type_t attachsql_column_type_t;

enum attachsql_column_flags_t
{
  ATTACHSQL_COLUMN_FLAGS_NONE=              0,
  ATTACHSQL_COLUMN_FLAGS_NOT_NULL=          (1 << 0),
  ATTACHSQL_COLUMN_FLAGS_PRIMARY_KEY=       (1 << 1),
  ATTACHSQL_COLUMN_FLAGS_UNIQUE_KEY=        (1 << 2),
  ATTACHSQL_COLUMN_FLAGS_MULTIPLE_KEY=      (1 << 3),
  ATTACHSQL_COLUMN_FLAGS_BLOB=              (1 << 4), // Deprecated in MySQL
  ATTACHSQL_COLUMN_FLAGS_UNSIGNED=          (1 << 5),
  ATTACHSQL_COLUMN_FLAGS_ZEROFILL=          (1 << 6),
  ATTACHSQL_COLUMN_FLAGS_BINARY=            (1 << 7),
  ATTACHSQL_COLUMN_FLAGS_ENUM=              (1 << 8),
  ATTACHSQL_COLUMN_FLAGS_AUTO_INCREMENT=    (1 << 9),
  ATTACHSQL_COLUMN_FLAGS_TIMESTAMP=         (1 << 10), // Deprecated in MySQL
  ATTACHSQL_COLUMN_FLAGS_SET=               (1 << 11),
  ATTACHSQL_COLUMN_FLAGS_NO_DEFAULT_VALUE=  (1 << 12),
  ATTACHSQL_COLUMN_FLAGS_ON_UPDATE_NOW=     (1 << 13), // Undocumented in MySQL
  ATTACHSQL_COLUMN_FLAGS_PART_KEY=          (1 << 14), // Undocumented in MySQL
  ATTACHSQL_COLUMN_FLAGS_NUM=               (1 << 15),
  ATTACHSQL_COLUMN_FLAGS_GROUP=             (1 << 15), // NUM and GROUP are both this // Undocumented in MySQL
  ATTACHSQL_COLUMN_FLAGS_UNIQUE=            (1 << 16), // Undocumented in MySQL
  ATTACHSQL_COLUMN_FLAGS_BINCMP=            (1 << 17), // Undocumented in MySQL
  ATTACHSQL_COLUMN_FLAGS_GET_FIELD_COLUMNS= (1 << 18), // Undocumented in MySQL
  ATTACHSQL_COLUMN_FLAGS_IN_PART_FUNC=      (1 << 19), // Undocumented in MySQL
  ATTACHSQL_COLUMN_FLAGS_IN_ADD_INDEX=      (1 << 20), // Undocumented in MySQL
  ATTACHSQL_COLUMN_FLAGS_RENAMED=           (1 << 21)  // Undocumented in MySQL
};

typedef enum attachsql_column_flags_t attachsql_column_flags_t;

enum attachsql_query_parameter_type_t
{
  ATTACHSQL_ESCAPE_TYPE_NONE,
  ATTACHSQL_ESCAPE_TYPE_CHAR,
  ATTACHSQL_ESCAPE_TYPE_CHAR_LIKE,
  ATTACHSQL_ESCAPE_TYPE_INT,
  ATTACHSQL_ESCAPE_TYPE_BIGINT,
  ATTACHSQL_ESCAPE_TYPE_FLOAT,
  ATTACHSQL_ESCAPE_TYPE_DOUBLE
};

typedef enum attachsql_query_parameter_type_t attachsql_query_parameter_type_t;

struct attachsql_query_parameter_st
{
  attachsql_query_parameter_type_t type;
  void *data;
  size_t length;
  bool is_unsigned;
};

typedef struct attachsql_query_parameter_st attachsql_query_parameter_st;

struct attachsql_query_column_st
{
  char *schema;
  char *table;
  char *origin_table;
  char *column;
  char *origin_column;
  uint16_t charset;
  uint32_t length;
  attachsql_column_type_t type;
  attachsql_column_flags_t flags;
  uint8_t decimals;
  char *default_value;
  size_t default_size;
};

typedef struct attachsql_query_column_st attachsql_query_column_st;

struct attachsql_query_row_st
{
  char *data;
  size_t length;
};

typedef struct attachsql_query_row_st attachsql_query_row_st;

ASQL_API
attachsql_error_st *attachsql_query(attachsql_connect_t *con, size_t length, const char *statement, uint16_t parameter_count, attachsql_query_parameter_st *parameters);

ASQL_API
void attachsql_query_close(attachsql_connect_t *con);

ASQL_API
uint16_t attachsql_query_column_count(attachsql_connect_t *con);

ASQL_API
attachsql_query_column_st *attachsql_query_column_get(attachsql_connect_t *con, uint16_t column);

ASQL_API
attachsql_query_row_st *attachsql_query_row_get(attachsql_connect_t *con, attachsql_error_st **error);

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


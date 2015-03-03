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

/* 5 bytes plus NUL terminator */
#define ATTACHSQL_SQLSTATE_SIZE 6
#define ATTACHSQL_MESSAGE_SIZE 256

struct attachsql_connect_t;
typedef struct attachsql_connect_t attachsql_connect_t;

enum attachsql_events_t
{
  ATTACHSQL_EVENT_NONE,
  ATTACHSQL_EVENT_CONNECTED,
  ATTACHSQL_EVENT_ERROR,
  ATTACHSQL_EVENT_EOF,
  ATTACHSQL_EVENT_ROW_READY
};

typedef enum attachsql_events_t attachsql_events_t;

enum attachsql_return_t
{
  ATTACHSQL_RETURN_NONE,
  ATTACHSQL_RETURN_NOT_CONNECTED,
  ATTACHSQL_RETURN_CONNECTING,
  ATTACHSQL_RETURN_PROCESSING,
  ATTACHSQL_RETURN_ROW_READY,
  ATTACHSQL_RETURN_ERROR,
  ATTACHSQL_RETURN_EOF
};

typedef enum attachsql_return_t attachsql_return_t;

enum attachsql_options_t
{
  ATTACHSQL_OPTION_NONE,
  ATTACHSQL_OPTION_COMPRESS,
  ATTACHSQL_OPTION_FOUND_ROWS,
  ATTACHSQL_OPTION_IGNORE_SIGPIPE,
  ATTACHSQL_OPTION_INTERACTIVE,
  ATTACHSQL_OPTION_LOCAL_FILES,
  ATTACHSQL_OPTION_MULTI_STATEMENTS,
  ATTACHSQL_OPTION_NO_SCHEMA,
  ATTACHSQL_OPTION_SSL_NO_VERIFY,
  ATTACHSQL_OPTION_SEMI_BLOCKING
};

typedef enum attachsql_options_t attachsql_options_t;

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
  ATTACHSQL_COLUMN_TYPE_VARCHAR=     0x0f,
  ATTACHSQL_COLUMN_TYPE_BIT=         0x10,
  ATTACHSQL_COLUMN_TYPE_NEWDECIMAL=  0xf6,
  ATTACHSQL_COLUMN_TYPE_ENUM=        0xf7,
  ATTACHSQL_COLUMN_TYPE_SET=         0xf8,
  ATTACHSQL_COLUMN_TYPE_TINY_BLOB=   0xf9,
  ATTACHSQL_COLUMN_TYPE_MEDIUM_BLOB= 0xfa,
  ATTACHSQL_COLUMN_TYPE_LONG_BLOB=   0xfb,
  ATTACHSQL_COLUMN_TYPE_BLOB=        0xfc,
  ATTACHSQL_COLUMN_TYPE_VARSTRING=   0xfd,
  ATTACHSQL_COLUMN_TYPE_STRING=      0xfe,
  ATTACHSQL_COLUMN_TYPE_GEOMETRY=    0xff,
  ATTACHSQL_COLUMN_TYPE_ERROR=       0xffff
};

typedef enum attachsql_column_type_t attachsql_column_type_t;

enum attachsql_error_level_t
{
  ATTACHSQL_ERROR_LEVEL_NOTICE,
  ATTACHSQL_ERROR_LEVEL_WARNING,
  ATTACHSQL_ERROR_LEVEL_ERROR
};

typedef enum attachsql_error_level_t attachsql_error_level_t;

/* Start at 2000 to line up with MySQL client */
enum attachsql_error_codes_t
{
  ATTACHSQL_ERROR_CODE_UNKNOWN=                   2000,
  /* 2001 not going to use */
  ATTACHSQL_ERROR_CODE_CONNECT=                   2002,
  /* 2003 & 2004 not going to use */
  ATTACHSQL_ERROR_CODE_HOST_UNKNOWN=              2005,
  ATTACHSQL_ERROR_CODE_SERVER_GONE=               2006,
  ATTACHSQL_ERROR_CODE_VERSION_MISMATCH=          2007,
  ATTACHSQL_ERROR_CODE_ALLOC=                     2008,
  ATTACHSQL_ERROR_CODE_WRONG_HOST_INFO=           2009,
  /* 2010 & 2011 are not errors */
  ATTACHSQL_ERROR_CODE_HANDSHAKE=                 2012,
  ATTACHSQL_ERROR_CODE_SERVER_LOST=               2013,
  ATTACHSQL_ERROR_CODE_OUT_OF_SYNC=               2014,
  /* 2015 is not an error */
  /* 2016 - 2018 is named pipe, not supported */
  /* 2019 is charset path, not supported */
  ATTACHSQL_ERROR_CODE_OVERSIZED_PACKET=          2020,
  /* 2021 is not an error */
  ATTACHSQL_ERROR_CODE_SLAVE_STATUS=              2022,
  ATTACHSQL_ERROR_CODE_SLAVE_HOSTS=               2023,
  ATTACHSQL_ERROR_CODE_SLAVE_CONNECT=             2024,
  ATTACHSQL_ERROR_CODE_MASTER_CONNECT=            2025,
  ATTACHSQL_ERROR_CODE_SSL_CONNECT=               2026,
  ATTACHSQL_ERROR_CODE_MALFORMED_PACKET=          2027,
  /* 2028 is invalid license, not supported */
  ATTACHSQL_ERROR_CODE_NULL_POINTER=              2029,
  ATTACHSQL_ERROR_CODE_STATEMENT_NOT_PREPARED=    2030,
  ATTACHSQL_ERROR_CODE_PARAMS_NOT_BOUND=          2031,
  ATTACHSQL_ERROR_CODE_DATA_TRUNCATED=            2032,
  ATTACHSQL_ERROR_CODE_NO_PARAMETERS_EXIST=       2033,
  ATTACHSQL_ERROR_CODE_INVALID_PARAMETER_NUMBER=  2034,
  ATTACHSQL_ERROR_CODE_INVALID_BUFFER_USE=        2035,
  ATTACHSQL_ERROR_CODE_INVALID_PARAMETER_TYPE=    2036,
  /* 2037 is not an error */
  /* 2038 - 2046 is SHM, not supported */
  ATTACHSQL_ERROR_CODE_UNKNOWN_PROTOCOL=          2047,
  ATTACHSQL_ERROR_CODE_INVALID_CON_HANDLE=        2048,
  /* 2049 - 2050 are not errors */
  ATTACHSQL_ERROR_CODE_NO_DATA=                   2051,
  ATTACHSQL_ERROR_CODE_NO_METADATA=               2052,
  ATTACHSQL_ERROR_CODE_NO_RESULT_SET=             2053,
  ATTACHSQL_ERROR_CODE_NOT_IMPLEMENTED=           2054,
  ATTACHSQL_ERROR_CODE_SERVER_LOST_EXTENDED=      2055,
  /* 2056 is not an error */
  ATTACHSQL_ERROR_CODE_NEW_STMT_METADATA=         2057,
  ATTACHSQL_ERROR_CODE_ALREADY_CONNECTED=         2058,
  ATTACHSQL_ERROR_CODE_AUTH_PLUGIN_LOAD=          2059,
  ATTACHSQL_ERROR_CODE_CONNECTION_ATTR=           2060,
  ATTACHSQL_ERROR_CODE_AUTH_PLUGIN=               2061,
  /* Our own errors starting at 3000 */
  ATTACHSQL_ERROR_CODE_PARAMETER=                 3000,
  ATTACHSQL_ERROR_CODE_BUFFERED_MODE=             3001,
  ATTACHSQL_ERROR_CODE_NO_SSL=                    3002,
  ATTACHSQL_ERROR_CODE_SSL=                       3003
};

typedef enum attachsql_error_codes_t attachsql_error_codes_t;

typedef struct attachsql_error_t attachsql_error_t;

struct attachsql_group_t;
typedef struct attachsql_group_t attachsql_group_t;

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

typedef void (attachsql_callback_fn)(attachsql_connect_t *con, attachsql_events_t events, void *context, attachsql_error_t *error);

#ifdef __cplusplus
}
#endif


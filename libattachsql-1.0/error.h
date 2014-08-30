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

struct attachsql_error_st
{
  int code;
  char msg[ATTACHSQL_MESSAGE_SIZE];
  char sqlstate[ATTACHSQL_SQLSTATE_SIZE];
  attachsql_error_level_t level;
};

typedef struct attachsql_error_st attachsql_error_st;

ASQL_API
void attachsql_error_free(attachsql_error_st *err);

#ifdef __cplusplus
}
#endif


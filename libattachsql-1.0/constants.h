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

typedef void (attachsql_callback_fn)(attachsql_connect_t *con, attachsql_events_t events, void *context);

enum attachsql_return_t
{
  ATTACHSQL_RETURN_NONE,
  ATTACHSQL_RETURN_NOT_CONNECTED,
  ATTACHSQL_RETURN_CONNECTING,
  ATTACHSQL_RETURN_IDLE,
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
  ATTACHSQL_OPTION_NO_SCHEMA
};

typedef enum attachsql_options_t attachsql_options_t;

#ifdef __cplusplus
}
#endif

